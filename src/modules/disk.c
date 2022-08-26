#include "fastfetch.h"
#include "common/printing.h"
#include "common/parsing.h"

#include <sys/statvfs.h>

#define FF_DISK_MODULE_NAME "Disk"
#define FF_DISK_NUM_FORMAT_ARGS 4

static void getKey(FFinstance* instance, FFstrbuf* key, const char* folderPath, bool showFolderPath)
{
    if(instance->config.disk.key.length == 0)
    {
        if(showFolderPath)
            ffStrbufAppendF(key, FF_DISK_MODULE_NAME" (%s)", folderPath);
        else
            ffStrbufSetS(key, FF_DISK_MODULE_NAME);
    }
    else
    {
        ffParseFormatString(key, &instance->config.disk.key, 1, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRING, folderPath}
        });
    }
}

static void printStatvfs(FFinstance* instance, FFstrbuf* key, struct statvfs* fs)
{
    uint64_t total = fs->f_blocks * fs->f_frsize;
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
}

static void printStatvfsCreateKey(FFinstance* instance, const char* folderPath, struct statvfs* fs)
{
    FF_STRBUF_CREATE(key);
    getKey(instance, &key, folderPath, true);
    printStatvfs(instance, &key, fs);
    ffStrbufDestroy(&key);
}

static void printFolder(FFinstance* instance, const char* folderPath)
{
    FF_STRBUF_CREATE(key);
    getKey(instance, &key, folderPath, true);

    struct statvfs fs;
    int ret = statvfs(folderPath, &fs);
    if(ret != 0 && instance->config.disk.outputFormat.length == 0)
    {
        ffPrintErrorString(instance, key.chars, 0, NULL, &instance->config.disk.errorFormat, "statvfs(\"%s\", &fs) != 0 (%i)", folderPath, ret);
        ffStrbufDestroy(&key);
        return;
    }

    printStatvfs(instance, &key, &fs);
    ffStrbufDestroy(&key);
}

void ffPrintDisk(FFinstance* instance)
{
    if(instance->config.diskFolders.length == 0)
    {
        struct statvfs fsRoot;
        int rootRet = statvfs("/", &fsRoot);

        struct statvfs fsHome;
        int homeRet = statvfs(FASTFETCH_TARGET_DIR_HOME, &fsHome);

        if(rootRet != 0 && homeRet != 0)
        {
            FF_STRBUF_CREATE(key);
            getKey(instance, &key, "", false);
            ffPrintErrorString(instance, key.chars, 0, NULL, &instance->config.disk.errorFormat, "statvfs failed for both / and " FASTFETCH_TARGET_DIR_HOME);
            ffStrbufDestroy(&key);
            return;
        }

        if(rootRet == 0)
            printStatvfsCreateKey(instance, "/", &fsRoot);

        if(homeRet == 0 && (rootRet != 0 || fsRoot.f_fsid != fsHome.f_fsid))
            printStatvfsCreateKey(instance, FASTFETCH_TARGET_DIR_HOME, &fsHome);
    }
    else
    {
        ffStrbufTrim(&instance->config.diskFolders, ':');

        if(instance->config.diskFolders.length == 0)
        {
            FF_STRBUF_CREATE(key);
            getKey(instance, &key, "", false);
            ffPrintErrorString(instance, key.chars, 0, NULL, &instance->config.disk.errorFormat, "Custom disk folders string doesn't contain any folders");
            ffStrbufDestroy(&key);
            return;
        }

        uint32_t startIndex = 0;
        while (startIndex < instance->config.diskFolders.length)
        {
            uint32_t colonIndex = ffStrbufNextIndexC(&instance->config.diskFolders, startIndex, ':');
            instance->config.diskFolders.chars[colonIndex] = '\0';

            printFolder(instance, instance->config.diskFolders.chars + startIndex);

            startIndex = colonIndex + 1;
        }
    }
}
