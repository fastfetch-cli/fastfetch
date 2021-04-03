#include "fastfetch.h"

void ffPrintOS(FFinstance* instance)
{
    if(ffPrintCachedValue(instance, &instance->config.osKey, "OS"))
        return;

    FILE* osRelease = fopen("/etc/os-release", "r");

    if(osRelease == NULL)
        osRelease = fopen("/usr/lib/os-release", "r");

    if(osRelease == NULL)
    {
        ffPrintError(instance, &instance->config.osKey, "OS", "couldn't read /etc/os-release nor /usr/lib/os-release");
        return;
    }

    // Documentation of the fields:
    // https://www.freedesktop.org/software/systemd/man/os-release.html

    char name[128];            name[0] = '\0';
    char prettyName[128];      prettyName[0] = '\0';
    char id[128];              id[0] = '\0';
    char idLike[128];          idLike[0] = '\0';
    char variant[128];         variant[0] = '\0';
    char variantId[128];       variantId[0] = '\0';
    char version[128];         version[0] = '\0';
    char versionId[128];       versionId[0] = '\0';
    char versionCodename[128]; versionCodename[0] = '\0';
    char buildId[128];         buildId[0] = '\0';

    char* line = NULL;
    size_t len = 0;

    while (getline(&line, &len, osRelease) != -1)
    {
        sscanf(line, "NAME=%[^\n]", name);
        sscanf(line, "NAME=\"%[^\"]+", name);
        sscanf(line, "PRETTY_NAME=%[^\n]", prettyName);
        sscanf(line, "PRETTY_NAME=\"%[^\"]+", prettyName);
        sscanf(line, "ID=%[^\n]", id);
        sscanf(line, "ID=\"%[^\"]+", id);
        sscanf(line, "ID_LIKE=%[^\n]", idLike);
        sscanf(line, "ID_LIKE=\"%[^\"]+", idLike);
        sscanf(line, "VARIANT=%[^\n]", variant);
        sscanf(line, "VARIANT=\"%[^\"]+", variant);
        sscanf(line, "VARIANT_ID=%[^\n]", variantId);
        sscanf(line, "VARIANT_ID=\"%[^\"]+", variantId);
        sscanf(line, "VERSION=%[^\n]", version);
        sscanf(line, "VERSION=\"%[^\"]+", version);
        sscanf(line, "VERSION_ID=%[^\n]", versionId);
        sscanf(line, "VERSION_ID=\"%[^\"]+", versionId);
        sscanf(line, "VERSION_CODENAME=%[^\n]", versionCodename);
        sscanf(line, "VERSION_CODENAME=\"%[^\"]+", versionCodename);
        sscanf(line, "BUILD_ID=%[^\n]", buildId);
        sscanf(line, "BUILD_ID=\"%[^\"]+", buildId);
    }

    if(line != NULL)
        free(line);

    fclose(osRelease);

    FF_STRBUF_CREATE(os);

    if(instance->config.osFormat.length == 0)
    {
        if(prettyName[0] != '\0')
        {
            ffStrbufAppendS(&os, prettyName);
        }
        else if(name[0] == '\0' && id[0] == '\0')
        {
            ffStrbufAppendS(&os, instance->state.utsname.sysname);
        }
        else
        {
            if(name[0] != '\0')
                ffStrbufAppendS(&os, name);
            else
                ffStrbufAppendS(&os, id);

            if(version[0] != '\0')
            {
                ffStrbufAppendC(&os, ' ');
                ffStrbufAppendS(&os, version);
            }
            else
            {
                if(versionId[0] != '\0')
                {
                    ffStrbufAppendC(&os, ' ');
                    ffStrbufAppendS(&os, versionId);
                }

                if(variant[0] != '\0')
                {
                    ffStrbufAppendS(&os, " (");
                    ffStrbufAppendS(&os, variant);
                    ffStrbufAppendC(&os, ')');
                }
                else if(variantId[0] != '\0')
                {
                    ffStrbufAppendS(&os, " (");
                    ffStrbufAppendS(&os, variantId);
                    ffStrbufAppendC(&os, ')');
                }
            }
        }

        ffStrbufAppendC(&os, ' ');
        ffStrbufAppendS(&os, instance->state.utsname.machine);
    }
    else
    {
        ffParseFormatString(&os, &instance->config.osFormat, 12,
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, instance->state.utsname.sysname},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, name},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, prettyName},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, id},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, idLike},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, variant},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, variantId},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, version},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, versionId},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, versionCodename},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, buildId},
            (FFformatarg){FF_FORMAT_ARG_TYPE_STRING, instance->state.utsname.machine}
        );
    }

    ffPrintAndSaveCachedValue(instance, &instance->config.osKey, "OS", &os);
    ffStrbufDestroy(&os);
}
