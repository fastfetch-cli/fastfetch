#include "board.h"
#include "common/settings.h"
#include "util/smbiosHelper.h"

const char* ffDetectBoard(FFBoardResult* result)
{
    ffSettingsGetFreeBSDKenv("smbios.planar.product", &result->name);
    ffCleanUpSmbiosValue(&result->name);
    ffSettingsGetFreeBSDKenv("smbios.planar.serial", &result->serial);
    ffCleanUpSmbiosValue(&result->serial);
    ffSettingsGetFreeBSDKenv("smbios.planar.maker", &result->vendor);
    ffCleanUpSmbiosValue(&result->vendor);
    ffSettingsGetFreeBSDKenv("smbios.planar.version", &result->version);
    ffCleanUpSmbiosValue(&result->version);
    return NULL;
}
