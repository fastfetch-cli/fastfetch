#include "os.h"
#include <OS.h>
#include <image.h>

// https://github.com/haiku/haiku/blob/e63683b2fb337d2034059a7e053c170eaf978142/headers/os/BeBuild.h#L36
#ifndef B_HAIKU_VERSION_1_PRE_BETA_6
#define B_HAIKU_VERSION_1_PRE_BETA_6	0x00000901
#endif
#ifndef B_HAIKU_VERSION_1_BETA_6
#define B_HAIKU_VERSION_1_BETA_6		0x00000A00
#endif
#ifndef B_HAIKU_VERSION_1_PRE_BETA_7
#define B_HAIKU_VERSION_1_PRE_BETA_7	0x00000A01
#endif
#ifndef B_HAIKU_VERSION_1_BETA_7
#define B_HAIKU_VERSION_1_BETA_7		0x00000B00
#endif
#ifndef B_HAIKU_VERSION_1_PRE_BETA_8
#define B_HAIKU_VERSION_1_PRE_BETA_8	0x00000B01
#endif
#ifndef B_HAIKU_VERSION_1_BETA_8
#define B_HAIKU_VERSION_1_BETA_8		0x00000C00
#endif
#ifndef B_HAIKU_VERSION_1_PRE_BETA_9
#define B_HAIKU_VERSION_1_PRE_BETA_9	0x00000C01
#endif
#ifndef B_HAIKU_VERSION_1_BETA_9
#define B_HAIKU_VERSION_1_BETA_9		0x00000D00
#endif
#ifndef B_HAIKU_VERSION_1
#define B_HAIKU_VERSION_1				0x00010000
#endif

void ffDetectOSImpl(FFOSResult* os)
{
    system_info sys;

    ffStrbufSetStatic(&os->name, "Haiku");

    ffStrbufSetStatic(&os->id, "haiku");

    image_info image;
    int32 cookie = 0;
    while (get_next_image_info(B_SYSTEM_TEAM, &cookie, &image) == B_OK)
    {
        if (image.api_version > 0)
        {
            switch (image.api_version)
            {
                #define FF_TEST_HAIKU_VERSION(versionNum, versionStr) \
                    case B_HAIKU_VERSION_ ## versionNum: \
                        ffStrbufSetStatic(&os->version, versionStr); \
                        break;

                FF_TEST_HAIKU_VERSION(1_ALPHA_1, "R1A1")
                FF_TEST_HAIKU_VERSION(1_PRE_ALPHA_2, "R1A2-")
                FF_TEST_HAIKU_VERSION(1_ALPHA_2, "R1A2")
                FF_TEST_HAIKU_VERSION(1_PRE_ALPHA_3, "R1A3-")
                FF_TEST_HAIKU_VERSION(1_ALPHA_3, "R1A3")
                FF_TEST_HAIKU_VERSION(1_PRE_ALPHA_4, "R1A4-")
                FF_TEST_HAIKU_VERSION(1_ALPHA_4, "R1A4")
                FF_TEST_HAIKU_VERSION(1_PRE_BETA_1, "R1B1-")
                FF_TEST_HAIKU_VERSION(1_BETA_1, "R1B1")
                FF_TEST_HAIKU_VERSION(1_PRE_BETA_2, "R1B2-")
                FF_TEST_HAIKU_VERSION(1_BETA_2, "R1B2")
                FF_TEST_HAIKU_VERSION(1_PRE_BETA_3, "R1B3-")
                FF_TEST_HAIKU_VERSION(1_BETA_3, "R1B3")
                FF_TEST_HAIKU_VERSION(1_PRE_BETA_4, "R1B4-")
                FF_TEST_HAIKU_VERSION(1_BETA_4, "R1B4")
                FF_TEST_HAIKU_VERSION(1_PRE_BETA_5, "R1B5-")
                FF_TEST_HAIKU_VERSION(1_BETA_5, "R1B5")
                FF_TEST_HAIKU_VERSION(1_PRE_BETA_6, "R1B6-")
                FF_TEST_HAIKU_VERSION(1_BETA_6, "R1B6")
                FF_TEST_HAIKU_VERSION(1_PRE_BETA_7, "R1B7-")
                FF_TEST_HAIKU_VERSION(1_BETA_7, "R1B7")
                FF_TEST_HAIKU_VERSION(1_PRE_BETA_8, "R1B8-")
                FF_TEST_HAIKU_VERSION(1_BETA_8, "R1B8")
                FF_TEST_HAIKU_VERSION(1_PRE_BETA_9, "R1B9-")
                FF_TEST_HAIKU_VERSION(1_BETA_9, "R1B9")
                FF_TEST_HAIKU_VERSION(1, "R1")
            }

            break;
        }
    }

    if (!os->version.length)
    {
        if (get_system_info(&sys) == B_OK)
            ffStrbufAppendF(&os->version, "R%ldx", sys.kernel_version);
    }
}
