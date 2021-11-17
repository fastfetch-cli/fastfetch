#include "fastfetch.h"

#include <sys/statvfs.h>

#define FF_DISK_MODULE_NAME "Disk"
#define FF_DISK_NUM_FORMAT_ARGS 4

static void getKey(FFinstance* instance, FFstrbuf* key, const char* folderPath, bool showFolderPath)
{
    if(instance->config.diskKey.length == 0)
    {
        if(showFolderPath)
            ffStrbufAppendF(key, FF_DISK_MODULE_NAME" (%s)", folderPath);
        else
            ffStrbufSetS(key, FF_DISK_MODULE_NAME);
    }
    else
    {
        ffParseFormatString(key, &instance->config.diskKey, NULL, 1, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_STRING, folderPath}
        });
    }
}

static void printStatvfs(FFinstance* instance, FFstrbuf* key, struct statvfs* fs)
{
    const uint32_t GB = 1024 * 1024 * 1024;

    uint32_t total     = ((uint32_t) (fs->f_blocks * fs->f_frsize)) / GB;
    uint32_t available = ((uint32_t) (fs->f_bfree  * fs->f_frsize)) / GB;
    uint32_t used      = total - available;
    uint8_t percentage = (uint8_t) ((used / (double) total) * 100.0);

    uint32_t files = (uint32_t) (fs->f_files - fs->f_ffree);

    if(instance->config.diskFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, key->chars, 0, NULL);
        printf("%uGB / %uGB (%u%%)\n", used, total, percentage);
    }
    else
    {
        ffPrintFormatString(instance, key->chars, 0, NULL, &instance->config.diskFormat, NULL, FF_DISK_NUM_FORMAT_ARGS, (FFformatarg[]){
            {FF_FORMAT_ARG_TYPE_UINT, &used},
            {FF_FORMAT_ARG_TYPE_UINT, &total},
            {FF_FORMAT_ARG_TYPE_UINT, &files},
            {FF_FORMAT_ARG_TYPE_UINT8, &percentage}
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
    if(ret != 0 && instance->config.diskFormat.length == 0)
    {
        ffPrintError(instance, key.chars, 0, NULL, &instance->config.diskFormat, FF_DISK_NUM_FORMAT_ARGS, "statvfs(\"%s\", &fs) != 0 (%i)", folderPath, ret);
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
        int homeRet = statvfs("/home", &fsHome);

        if(rootRet != 0 && homeRet != 0)
        {
            FF_STRBUF_CREATE(key);
            getKey(instance, &key, "", false);
            ffPrintError(instance, key.chars, 0, NULL, &instance->config.diskFormat, FF_DISK_NUM_FORMAT_ARGS, "statvfs failed for both / and /home");
            ffStrbufDestroy(&key);
            return;
        }

        if(rootRet == 0)
            printStatvfsCreateKey(instance, "/", &fsRoot);

        if(homeRet == 0 && (rootRet != 0 || fsRoot.f_fsid != fsHome.f_fsid))
            printStatvfsCreateKey(instance, "/home", &fsHome);
    }
    else
    {
        ffStrbufTrim(&instance->config.diskFolders, ':');

        if(instance->config.diskFolders.length == 0)
        {
            FF_STRBUF_CREATE(key);
            getKey(instance, &key, "", false);
            ffPrintError(instance, key.chars, 0, NULL, &instance->config.diskFormat, FF_DISK_NUM_FORMAT_ARGS, "Custom disk folders string doesn't contain any folders");
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
