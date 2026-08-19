#include "wifi.h"
#include "common/io.h"
#include "common/mallocHelper.h"

#include <dirent.h>
#include <fcntl.h>
#include <stropts.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <inet/wifi_ioctl.h>

static bool getWifiParam(int fd, uint32_t id, size_t minLength, wldp_t* buffer) {
	memset(buffer, 0, MAX_BUF_LEN);
	buffer->wldp_type = NET_802_11;
	buffer->wldp_id = id;

	struct strioctl request = {
		.ic_cmd = WLAN_GET_PARAM,
		.ic_timout = 0,
		.ic_len = MAX_BUF_LEN,
		.ic_dp = (char*) buffer,
	};

	return ioctl(fd, I_STR, &request) == 0
		&& buffer->wldp_result == WL_SUCCESS
		&& buffer->wldp_length >= WIFI_BUF_OFFSET + minLength
		&& buffer->wldp_length <= MAX_BUF_LEN;
}

static bool getWifiValue(int fd, uint32_t id, void* value, size_t valueSize, wldp_t* buffer) {
	if (!getWifiParam(fd, id, valueSize, buffer)) {
		return false;
	}

	memcpy(value, buffer->wldp_buf, valueSize);
	return true;
}

static void initWifiResult(FFWifiResult* item, const char* name) {
	ffStrbufInitS(&item->inf.description, name);
	ffStrbufInit(&item->inf.status);
	ffStrbufInit(&item->inf.driver);
	ffStrbufInit(&item->conn.status);
	ffStrbufInit(&item->conn.ssid);
	ffStrbufInit(&item->conn.bssid);
	ffStrbufInit(&item->conn.protocol);
	ffStrbufInit(&item->conn.security);
	item->conn.signalQuality = -DBL_MAX;
	item->conn.rxRate = -DBL_MAX;
	item->conn.txRate = -DBL_MAX;
	item->conn.channel = 0;
	item->conn.channelWidth = 0;
	item->conn.frequency = 0;
}

static void setWifiProtocol(FFstrbuf* protocol, uint32_t subtype, bool htEnabled) {
	switch (subtype) {
		case WL_FHSS: ffStrbufSetStatic(protocol, "802.11 (FHSS)"); break;
		case WL_DSSS: ffStrbufSetStatic(protocol, "802.11 (DSSS)"); break;
		case WL_IRBASE: ffStrbufSetStatic(protocol, "802.11 (IR)"); break;
		case WL_OFDM: ffStrbufSetStatic(protocol, "802.11a"); break;
		case WL_HRDS: ffStrbufSetStatic(protocol, "802.11b"); break;
		case WL_ERP: ffStrbufSetStatic(protocol, htEnabled ? "802.11n (Wi-Fi 4)" : "802.11g"); break;
		default: break;
	}
}

static void setWifiSecurity(FFstrbuf* security, uint32_t encryption, bool haveAuthMode, uint32_t authMode) {
	switch (encryption) {
		case WL_NOENCRYPTION:
			ffStrbufSetStatic(security, "Insecure");
			break;
		case WL_ENC_WEP:
			ffStrbufSetStatic(security, "WEP");
			break;
		case WL_ENC_WPA:
			ffStrbufSetStatic(security, haveAuthMode && authMode == WL_SHAREDKEY ? "WPA Shared Key" : "WPA");
			break;
		default:
			break;
	}
}

const char* ffDetectWifi(FFlist* result) {
	FF_AUTO_CLOSE_DIR DIR* directory = opendir("/dev/wifi");
	if (!directory) {
		return nullptr;
	}

	FF_AUTO_FREE void* bufferMemory = malloc(MAX_BUF_LEN);
	if (!bufferMemory) {
		return "malloc() failed";
	}
	wldp_t* buffer = (wldp_t*) bufferMemory;

	struct dirent* entry;
	while ((entry = readdir(directory)) != nullptr) {
		size_t nameLength = strlen(entry->d_name);
		if (nameLength == 0 || entry->d_name[nameLength - 1] < '0' || entry->d_name[nameLength - 1] > '9') {
			continue;
		}

		char devicePath[PATH_MAX];
		int pathLength = snprintf(devicePath, sizeof(devicePath), "/dev/wifi/%s", entry->d_name);
		if (pathLength < 0 || (size_t) pathLength >= sizeof(devicePath)) {
			continue;
		}

		FF_AUTO_CLOSE_FD int fd = open(devicePath, O_RDWR);
		if (fd < 0) {
			continue;
		}

		FFWifiResult* item = FF_LIST_ADD(FFWifiResult, *result);
		initWifiResult(item, entry->d_name);

		wl_vendor_t vendor = {};
		wl_product_t product = {};
		if (getWifiParam(fd, WL_VENDOR_ID, sizeof(vendor), buffer)) {
			memcpy(vendor, buffer->wldp_buf, sizeof(vendor));
			vendor[sizeof(vendor) - 1] = '\0';
		}
		if (getWifiParam(fd, WL_PRODUCT_ID, sizeof(product), buffer)) {
			memcpy(product, buffer->wldp_buf, sizeof(product));
			product[sizeof(product) - 1] = '\0';
		}
		if (vendor[0]) {
			ffStrbufSetS(&item->inf.driver, vendor);
		}
		if (product[0]) {
			if (item->inf.driver.length) {
				ffStrbufAppendC(&item->inf.driver, ' ');
			}
			ffStrbufAppendS(&item->inf.driver, product);
		}

		wl_radio_t radio;
		if (getWifiValue(fd, WL_RADIO, &radio, sizeof(radio), buffer)) {
			ffStrbufSetStatic(&item->inf.status, radio ? "Up" : "Down");
		} else {
			ffStrbufSetStatic(&item->inf.status, "Unknown");
		}

		wl_linkstatus_t linkStatus;
		if (!getWifiValue(fd, WL_LINKSTATUS, &linkStatus, sizeof(linkStatus), buffer)) {
			ffStrbufSetStatic(&item->conn.status, "Unknown");
			continue;
		}
		if (linkStatus != WL_CONNECTED) {
			ffStrbufSetStatic(&item->conn.status, "Not connected");
			continue;
		}
		ffStrbufSetStatic(&item->conn.status, "Connected");

		wl_essid_t essid;
		if (getWifiValue(fd, WL_ESSID, &essid, sizeof(essid), buffer)
			&& essid.wl_essid_length < sizeof(essid.wl_essid_essid)
			&& essid.wl_essid_length <= MAX_ESSID_LENGTH - 1) {
			ffStrbufSetNS(&item->conn.ssid, essid.wl_essid_length, essid.wl_essid_essid);
		}

		wl_bssid_t bssid;
		if (getWifiValue(fd, WL_BSSID, bssid, sizeof(bssid), buffer)) {
			ffStrbufSetF(&item->conn.bssid, "%02X:%02X:%02X:%02X:%02X:%02X", bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);
		}

		wl_rssi_t rssi;
		if (getWifiValue(fd, WL_RSSI, &rssi, sizeof(rssi), buffer)) {
			item->conn.signalQuality = rssi >= MAX_RSSI ? 100.0 : ((double) rssi / MAX_RSSI) * 100.0;
		}

		wl_phy_conf_t phy;
		if (getWifiValue(fd, WL_PHY_CONFIG, &phy, sizeof(phy), buffer)) {
			uint32_t subtype = phy.wl_phy_dsss_conf.wl_dsss_subtype;
			switch (subtype) {
				case WL_FHSS:
					item->conn.channel = (uint16_t) phy.wl_phy_fhss_conf.wl_fhss_channel;
					setWifiProtocol(&item->conn.protocol, subtype, false);
					break;
				case WL_DSSS:
					item->conn.channel = (uint16_t) phy.wl_phy_dsss_conf.wl_dsss_channel;
					setWifiProtocol(&item->conn.protocol, subtype, false);
					break;
				case WL_OFDM:
					item->conn.frequency = (uint16_t) phy.wl_phy_ofdm_conf.wl_ofdm_frequency;
					item->conn.channel = ffWifiFreqToChannel(item->conn.frequency);
					setWifiProtocol(&item->conn.protocol, subtype, phy.wl_phy_ofdm_conf.wl_ofdm_ht_enabled);
					break;
				case WL_ERP:
					item->conn.channel = (uint16_t) phy.wl_phy_erp_conf.wl_erp_channel;
					setWifiProtocol(&item->conn.protocol, subtype, phy.wl_phy_erp_conf.wl_erp_ht_enabled);
					break;
				default:
					setWifiProtocol(&item->conn.protocol, subtype, false);
					break;
			}
		}

		wl_encryption_t encryption;
		wl_authmode_t authMode;
		bool haveEncryption = getWifiValue(fd, WL_ENCRYPTION, &encryption, sizeof(encryption), buffer);
		bool haveAuthMode = getWifiValue(fd, WL_AUTH_MODE, &authMode, sizeof(authMode), buffer);
		if (haveEncryption) {
			setWifiSecurity(&item->conn.security, encryption, haveAuthMode, authMode);
		}
	}

	return nullptr;
}
