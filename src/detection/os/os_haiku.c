#include "os.h"
#include <OS.h>
#include <image.h>

void ffDetectOSImpl(FFOSResult* os)
{
    ffStrbufSetStatic(&os->name, "Haiku");

    ffStrbufSetStatic(&os->id, "haiku");

    image_info image;
    int32 cookie = 0;
    while (get_next_image_info(B_SYSTEM_TEAM, &cookie, &image) == B_OK)
    {
        int32 ver = image.api_version;
        if (ver == 0) continue;

        // https://github.com/haiku/haiku/blob/e63683b2fb337d2034059a7e053c170eaf978142/headers/os/BeBuild.h#L36
        if (ver < B_HAIKU_VERSION_1_ALPHA_1)
        {
            switch (ver)
            {
                case B_HAIKU_VERSION_BEOS:
                    ffStrbufSetStatic(&os->version, "BEOS");
                    break;
                case B_HAIKU_VERSION_BONE:
                    ffStrbufSetStatic(&os->version, "BONE");
                    break;
                case B_HAIKU_VERSION_DANO:
                    ffStrbufSetStatic(&os->version, "DANO");
                    break;
            }
        }
        else
        {
            int32 relVer = ver / 0x10000;
            ver %= 0x10000;
            if (ver == 0)
            {
                ffStrbufSetF(&os->version, "R%d", relVer);
            }
            else
            {
                relVer++;

                bool isPre = !!(ver & 1);
                if (ver < B_HAIKU_VERSION_1_PRE_BETA_1)
                {
                    int32 alphaVer = ver / 0x100;
                    if (isPre)
                        ffStrbufSetF(&os->version, "R%dA%d-", relVer, alphaVer + 1);
                    else
                        ffStrbufSetF(&os->version, "R%dA%d", relVer, alphaVer);
                }
                else if (ver < 0x00010000 /* B_HAIKU_VERSION_1 */)
                {
                    int32 betaVer = (ver - B_HAIKU_VERSION_1_ALPHA_4) / 0x100;
                    if (isPre)
                        ffStrbufSetF(&os->version, "R%dB%d-", relVer, betaVer + 1);
                    else
                        ffStrbufSetF(&os->version, "R%dB%d", relVer, betaVer);
                }
            }
        }
    }

    if (!os->version.length)
    {
        system_info sys;
        if (get_system_info(&sys) == B_OK)
            ffStrbufAppendF(&os->version, "R%ldx", sys.kernel_version);
    }
}
