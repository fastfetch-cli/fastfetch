#include "board.h"
#include "common/settings.h"
#include "util/smbiosHelper.h"

const char* ffDetectBoard(FFBoardResult* result)
{
    ffSettingsGetFreeBSDKenv("smbios.planar.product", &result->name);
    ffCleanUpSmbiosValue(&result->name);
    ffSettingsGetFreeBSDKenv("smbios.planar.maker", &result->vendor);
    ffCleanUpSmbiosValue(&result->vendor);
    ffSettingsGetFreeBSDKenv("smbios.planar.version", &result->version);
    ffCleanUpSmbiosValue(&result->version);
    return NULL;
}
