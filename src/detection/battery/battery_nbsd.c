#include "fastfetch.h"
#include "battery.h"
#include "common/io/io.h"

#include <sys/envsys.h>
#include <prop/proplib.h>
#include <paths.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

const char* ffDetectBattery(FFBatteryOptions* options, FFlist* results)
{
    FF_AUTO_CLOSE_FD int fd = open(_PATH_SYSMON, O_RDONLY);
    if (fd < 0) return "open(_PATH_SYSMON, O_RDONLY) failed";

    prop_dictionary_t root = NULL;
    if (prop_dictionary_recv_ioctl(fd, ENVSYS_GETDICTIONARY, &root) < 0)
        return "prop_dictionary_recv_ioctl(ENVSYS_GETDICTIONARY) failed";

    prop_array_t array = prop_dictionary_get(root, "acpiacad0");
    if (!array)
        return "No key `acpibat0` in root dictionary";

    if (prop_array_count(array) != 2)
        return "Unexpect `acpibat0` data";

    prop_dictionary_t dict = prop_array_get(array, 0);
    if (prop_object_type(dict) != PROP_TYPE_DICTIONARY)
        return "Unexpect `acpibat0[0]`";

    //prop_array_t keys = prop_dictionary_all_keys(dict);
    //for (uint32_t i = 0; i < prop_array_count(keys); ++i)
    //    puts(prop_dictionary_keysym_value(prop_array_get(keys, i)));

    int acConnected = false;
    prop_dictionary_get_int(dict, "cur-value", &acConnected);

    // TODO: actually use acpibat0

    return acConnected ? "AC Connected" : "Discharging";
}
