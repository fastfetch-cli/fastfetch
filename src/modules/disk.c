#include "fastfetch.h"
#include "common/printing.h"
#include "common/parsing.h"

#include <sys/statvfs.h>

#define FF_DISK_MODULE_NAME "Disk"
#define FF_DISK_NUM_FORMAT_ARGS 4

static void createKey(FFinstance* instance, const char* folderPath, FFstrbuf* key)
{
    if(instance->config.disk.key.length == 0)
    {
        ffStrbufAppendS(key, FF_DISK_MODULE_NAME);
        if(folderPath != NULL)
            ffStrbufAppendF(key, " (%s)", folderPath);
    }
    else
    {
        ffParseFormatString(key, &instance->config.disk.key, 1, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRING, folderPath}
        });
    }
}

static void printStatvfs(FFinstance* instance, const FFstrbuf* key, const char* folderPath, struct statvfs* fs)
{
    uint64_t total = fs->f_blocks * fs->f_frsize;

    if(total == 0)
    {
        ffPrintErrorString(instance, key->chars, 0, NULL, &instance->config.disk.errorFormat, "statvfs for %s returned size 0", folderPath);
        return;
    }

    uint64_t used = total - (fs->f_bfree  * fs->f_frsize);
    uint32_t files = (uint32_t) (fs->f_files - fs->f_ffree);
    uint8_t percentage = (uint8_t) ((used / (long double) total) * 100.0);

    FFstrbuf usedPretty;
    ffStrbufInit(&usedPretty);
    ffParseSize(used, instance->config.binaryPrefixType, &usedPretty);

    FFstrbuf totalPretty;
    ffStrbufInit(&totalPretty);
    ffParseSize(total, instance->config.binaryPrefixType, &totalPretty);

    if(instance->config.disk.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, key->chars, 0, NULL);
        printf("%s / %s (%u%%)\n", usedPretty.chars, totalPretty.chars, percentage);
    }
    else
    {
        ffPrintFormatString(instance, key->chars, 0, NULL, &instance->config.disk.outputFormat, FF_DISK_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRBUF, &usedPretty},
            {FF_FORMAT_ARG_TYPE_STRBUF, &totalPretty},
            {FF_FORMAT_ARG_TYPE_UINT8, &percentage},
            {FF_FORMAT_ARG_TYPE_UINT, &files},
        });
    }

    ffStrbufDestroy(&totalPretty);
    ffStrbufDestroy(&usedPretty);
}

static void printFolderAutodetection(FFinstance* instance, const char* folderPath, struct statvfs* fs)
{
    FFstrbuf key;
    ffStrbufInit(&key);
    createKey(instance, folderPath, &key);
    printStatvfs(instance, &key, folderPath, fs);
    ffStrbufDestroy(&key);
}

static void printFoldersAutodetection(FFinstance* instance)
{
    struct statvfs fsRoot;
    int rootRet = statvfs(FASTFETCH_TARGET_DIR_ROOT"/", &fsRoot);

    if(rootRet != 0)
    {
        FFstrbuf key;
        ffStrbufInit(&key);
        createKey(instance, NULL, &key);
        ffPrintErrorString(instance, key.chars, 0, NULL, &instance->config.disk.errorFormat, "statvfs for / returned not zero: %i", rootRet);
        ffStrbufDestroy(&key);
        return;
    }

    //On MacOS statvfs seems to return different f_fsid for the same filesystem.
    //Since it isn't really possible to install /Users on a separate disk anyway, just never print it by default.
    #ifndef __APPLE__
        struct statvfs fsHome;
        int homeRet = statvfs(FASTFETCH_TARGET_DIR_HOME, &fsHome);
        bool printHome = homeRet == 0 && (fsRoot.f_fsid != fsHome.f_fsid);
    #else
        bool printHome = false;
    #endif // !__APPLE__

    printFolderAutodetection(instance, printHome ? FASTFETCH_TARGET_DIR_ROOT"/" : NULL, &fsRoot);

    #ifndef __APPLE__
    if(printHome)
        printFolderAutodetection(instance, FASTFETCH_TARGET_DIR_HOME, &fsHome);
    #endif // !__APPLE__
}

static void printFolderCustom(FFinstance* instance, const char* folderPath)
{
    FFstrbuf key;
    ffStrbufInit(&key);
    createKey(instance, folderPath, &key);

    struct statvfs fs;
    int ret = statvfs(folderPath, &fs);
    if(ret != 0)
    {
        ffPrintErrorString(instance, key.chars, 0, NULL, &instance->config.disk.errorFormat, "statvfs(\"%s\", &fs) != 0 (%i)", folderPath, ret);
        ffStrbufDestroy(&key);
        return;
    }

    printStatvfs(instance, &key, folderPath, &fs);
    ffStrbufDestroy(&key);
}

void ffPrintDisk(FFinstance* instance)
{
    ffStrbufTrim(&instance->config.diskFolders, ':');

    if(instance->config.diskFolders.length == 0)
    {
        printFoldersAutodetection(instance);
        return;
    }

    uint32_t startIndex = 0;
    while (startIndex < instance->config.diskFolders.length)
    {
        uint32_t colonIndex = ffStrbufNextIndexC(&instance->config.diskFolders, startIndex, ':');
        instance->config.diskFolders.chars[colonIndex] = '\0';

        printFolderCustom(instance, instance->config.diskFolders.chars + startIndex);

        startIndex = colonIndex + 1;
    }
}
