#include "fastfetch.h"

#include <pthread.h>

const FFOSResult* ffCalculateOS(FFinstance* instance)
{
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    static FFOSResult result;
    static bool init = false;
    pthread_mutex_lock(&mutex);
    if(init)
    {
        pthread_mutex_unlock(&mutex);
        return &result;
    }
    init = true;

    FILE* osRelease = fopen("/etc/os-release", "r");

    if(osRelease == NULL)
        osRelease = fopen("/usr/lib/os-release", "r");

    ffStrbufInitA(&result.error, 64);
    if(osRelease == NULL)
    {
        ffStrbufAppendS(&result.error, "couldn't read /etc/os-release nor /usr/lib/os-release");
        return &result;
    }

    ffStrbufInitA(&result.systemName, 256);
    ffStrbufInitA(&result.name, 256);
    ffStrbufInitA(&result.prettyName, 256);
    ffStrbufInitA(&result.id, 256);
    ffStrbufInitA(&result.idLike, 256);
    ffStrbufInitA(&result.variant, 256);
    ffStrbufInitA(&result.variantID, 256);
    ffStrbufInitA(&result.version, 256);
    ffStrbufInitA(&result.versionID, 256);
    ffStrbufInitA(&result.codename, 256);
    ffStrbufInitA(&result.buildID, 256);
    ffStrbufInitA(&result.architecture, 256);
    ffStrbufInitA(&result.error, 256);

    ffStrbufSetS(&result.systemName, instance->state.utsname.sysname);
    ffStrbufSetS(&result.architecture, instance->state.utsname.machine);

    char* line = NULL;
    size_t len = 0;

    // Documentation of the fields:
    // https://www.freedesktop.org/software/systemd/man/os-release.html
    while (getline(&line, &len, osRelease) != -1)
    {
        sscanf(line, "NAME=%[^\n]", result.name.chars);
        sscanf(line, "NAME=\"%[^\"]+", result.name.chars);
        sscanf(line, "PRETTY_NAME=%[^\n]", result.prettyName.chars);
        sscanf(line, "PRETTY_NAME=\"%[^\"]+", result.prettyName.chars);
        sscanf(line, "ID=%[^\n]", result.id.chars);
        sscanf(line, "ID=\"%[^\"]+", result.id.chars);
        sscanf(line, "ID_LIKE=%[^\n]", result.idLike.chars);
        sscanf(line, "ID_LIKE=\"%[^\"]+", result.idLike.chars);
        sscanf(line, "VARIANT=%[^\n]", result.variant.chars);
        sscanf(line, "VARIANT=\"%[^\"]+", result.variant.chars);
        sscanf(line, "VARIANT_ID=%[^\n]", result.variantID.chars);
        sscanf(line, "VARIANT_ID=\"%[^\"]+", result.variantID.chars);
        sscanf(line, "VERSION=%[^\n]", result.version.chars);
        sscanf(line, "VERSION=\"%[^\"]+", result.version.chars);
        sscanf(line, "VERSION_ID=%[^\n]", result.versionID.chars);
        sscanf(line, "VERSION_ID=\"%[^\"]+", result.versionID.chars);
        sscanf(line, "VERSION_CODENAME=%[^\n]", result.codename.chars);
        sscanf(line, "VERSION_CODENAME=\"%[^\"]+", result.codename.chars);
        sscanf(line, "BUILD_ID=%[^\n]", result.buildID.chars);
        sscanf(line, "BUILD_ID=\"%[^\"]+", result.buildID.chars);
    }

    ffStrbufRecalculateLength(&result.systemName);
    ffStrbufRecalculateLength(&result.name);
    ffStrbufRecalculateLength(&result.prettyName);
    ffStrbufRecalculateLength(&result.id);
    ffStrbufRecalculateLength(&result.idLike);
    ffStrbufRecalculateLength(&result.variant);
    ffStrbufRecalculateLength(&result.variantID);
    ffStrbufRecalculateLength(&result.version);
    ffStrbufRecalculateLength(&result.versionID);
    ffStrbufRecalculateLength(&result.codename);
    ffStrbufRecalculateLength(&result.buildID);
    ffStrbufRecalculateLength(&result.architecture);
    ffStrbufRecalculateLength(&result.error);

    if(line != NULL)
        free(line);

    fclose(osRelease);

    pthread_mutex_unlock(&mutex);

    return &result;
}
