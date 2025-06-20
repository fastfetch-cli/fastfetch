#include "battery.h"
#include "common/io/io.h"
#include "util/FFstrbuf.h"
#include "util/stringUtils.h"

#include <prop/prop_array.h>
#include <prop/prop_bool.h>
#include <prop/prop_dictionary.h>
#include <prop/prop_object.h>
#include <sys/envsys.h>
#include <prop/proplib.h>
#include <paths.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

const char* ffDetectBattery(FF_MAYBE_UNUSED FFBatteryOptions* options, FFlist* results)
{
    FF_AUTO_CLOSE_FD int fd = open(_PATH_SYSMON, O_RDONLY | O_CLOEXEC);
    if (fd < 0) return "open(_PATH_SYSMON, O_RDONLY | O_CLOEXEC) failed";

    prop_dictionary_t root = NULL;
    if (prop_dictionary_recv_ioctl(fd, ENVSYS_GETDICTIONARY, &root) < 0)
        return "prop_dictionary_recv_ioctl(ENVSYS_GETDICTIONARY) failed";

    bool acConnected = false;
    {
        prop_array_t acad = prop_dictionary_get(root, "acpiacad0");
        if (acad)
        {
            prop_dictionary_t dict = prop_array_get(acad, 0);
            prop_dictionary_get_uint8(dict, "cur-value", (uint8_t*) &acConnected);
        }
    }

    prop_object_iterator_t itKey = prop_dictionary_iterator(root);
    for (prop_dictionary_keysym_t key; (key = prop_object_iterator_next(itKey)) != NULL; )
    {
        if (!ffStrStartsWith(prop_dictionary_keysym_value(key), "acpibat")) continue;

        prop_array_t bat = prop_dictionary_get_keysym(root, key);
        uint32_t max = 0, curr = 0, dischargeRate = 0;
        bool charging = false, critical = false;
        prop_object_iterator_t iter = prop_array_iterator(bat);
        for (prop_dictionary_t dict; (dict = prop_object_iterator_next(iter)) != NULL;)
        {
            if (prop_object_type(dict) != PROP_TYPE_DICTIONARY)
                continue;

            const char* desc = NULL;
            if (!prop_dictionary_get_string(dict, "description", &desc))
                continue;

            if (ffStrEquals(desc, "present"))
            {
                int value = 0;
                if (prop_dictionary_get_int(dict, "cur-value", &value) && value == 0)
                    continue;
            }
            else if (ffStrEquals(desc, "charging"))
            {
                prop_dictionary_get_uint8(dict, "cur-value", (uint8_t*) &charging);
            }
            else if (ffStrEquals(desc, "charge"))
            {
                prop_dictionary_get_uint32(dict, "max-value", &max);
                prop_dictionary_get_uint32(dict, "cur-value", &curr);
                const char* state = NULL;
                if (prop_dictionary_get_string(dict, "state", &state) && ffStrEquals(state, "critical"))
                    critical = true;
            }
            else if (ffStrEquals(desc, "discharge rate"))
                prop_dictionary_get_uint(dict, "cur-value", &dischargeRate);
        }

        if (max > 0)
        {
            FFBatteryResult* battery = ffListAdd(results);
            battery->temperature = FF_BATTERY_TEMP_UNSET;
            battery->cycleCount = 0;
            ffStrbufInit(&battery->manufacturer);
            ffStrbufInit(&battery->modelName);
            ffStrbufInit(&battery->status);
            ffStrbufInit(&battery->technology);
            ffStrbufInit(&battery->serial);
            ffStrbufInit(&battery->manufactureDate);
            battery->timeRemaining = -1;

            battery->capacity = (double) curr / (double) max * 100.;
            if (charging)
                ffStrbufAppendS(&battery->status, "Charging, ");
            else if (dischargeRate)
            {
                ffStrbufAppendS(&battery->status, "Discharging, ");
                battery->timeRemaining = (int32_t)((double)curr / dischargeRate * 3600);
            }
            if (critical)
                ffStrbufAppendS(&battery->status, "Critical, ");
            if (acConnected)
                ffStrbufAppendS(&battery->status, "AC Connected");
            ffStrbufTrimRight(&battery->status, ' ');
            ffStrbufTrimRight(&battery->status, ',');
        }

        prop_object_iterator_release(iter);
    }
    prop_object_iterator_release(itKey);
    prop_object_release(root);

    return NULL;
}
