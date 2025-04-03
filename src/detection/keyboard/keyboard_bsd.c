#include "keyboard.h"
#include "common/io/io.h"

#include <stdio.h>
#include <fcntl.h>
#include <usbhid.h>
#include <sys/kbio.h>

#if __has_include(<dev/usb/usb_ioctl.h>)
    #include <dev/usb/usb_ioctl.h> // FreeBSD
#else
    #include <bus/u4b/usb_ioctl.h> // DragonFly
#endif

static const char* detectByIoctl(FFlist* devices)
{
    keyboard_info_t kbdInfo;
    if (ioctl(STDIN_FILENO, KDGKBINFO, &kbdInfo) != 0)
        return "ioctl(KDGKBINFO) failed";

    FFKeyboardDevice* device = (FFKeyboardDevice*) ffListAdd(devices);

    switch (kbdInfo.kb_type) {
        case KB_84:
            ffStrbufInitS(&device->name, "AT 84-key keyboard");
            break;
        case KB_101:
            ffStrbufInitS(&device->name, "AT 101/102-key keyboard");
            break;
        default:
            ffStrbufInitS(&device->name, "Unknown keyboard");
            break;
    }

    ffStrbufAppendF(&device->name, " (kbd%d)", kbdInfo.kb_index);

    ffStrbufInit(&device->serial);
    return NULL;
}

#define MAX_UHID_KBDS 64

static const char* detectByUsbhid(FFlist* devices)
{
    char path[16];
    for (int i = 0; i < MAX_UHID_KBDS; i++)
    {
        snprintf(path, ARRAY_SIZE(path), "/dev/uhid%d", i);
        FF_AUTO_CLOSE_FD int fd = open(path, O_RDONLY | O_CLOEXEC);
        if (fd < 0)
        {
            if (errno == ENOENT)
                break; // No more devices
            continue; // Device not found
        }

        report_desc_t repDesc = hid_get_report_desc(fd);
        if (!repDesc) continue;

        int reportId = hid_get_report_id(fd);

        struct hid_data* hData = hid_start_parse(repDesc, 0, reportId);
        if (hData)
        {
            struct hid_item hItem;
            while (hid_get_item(hData, &hItem) > 0)
            {
                if (HID_PAGE(hItem.usage) != 1 || HID_USAGE(hItem.usage) != 6) continue;

                struct usb_device_info di;
                if (ioctl(fd, USB_GET_DEVICEINFO, &di) != -1)
                {
                    FFKeyboardDevice* device = (FFKeyboardDevice*) ffListAdd(devices);
                    ffStrbufInitS(&device->serial, di.udi_serial);
                    ffStrbufInitS(&device->name, di.udi_product);
                }
            }
            hid_end_parse(hData);
        }

        hid_dispose_report_desc(repDesc);
    }

    return NULL;
}

const char* ffDetectKeyboard(FFlist* devices /* List of FFKeyboardDevice */)
{
    detectByUsbhid(devices);
    if (devices->length > 0)
        return NULL;
    return detectByIoctl(devices);
}
