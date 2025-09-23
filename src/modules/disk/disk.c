#include "common/printing.h"
#include "common/jsonconfig.h"
#include "common/percent.h"
#include "common/size.h"
#include "common/time.h"
#include "detection/disk/disk.h"
#include "modules/disk/disk.h"
#include "util/stringUtils.h"

#pragma GCC diagnostic ignored "-Wsign-conversion"

static void printDisk(FFDiskOptions* options, const FFDisk* disk, uint32_t index)
{
    FF_STRBUF_AUTO_DESTROY key = ffStrbufCreate();

    if(options->moduleArgs.key.length == 0)
    {
        ffStrbufSetF(&key, "%s (%s)", FF_DISK_MODULE_NAME, disk->mountpoint.chars);
    }
    else
    {
        FF_STRBUF_AUTO_DESTROY mountpointLink = ffStrbufCreate();
        FF_STRBUF_AUTO_DESTROY nameLink = ffStrbufCreate();
        #ifdef __linux__
        if (getenv("WSL_DISTRO_NAME") != NULL && getenv("WT_SESSION") != NULL)
        {
            if (ffStrbufEqualS(&disk->filesystem, "9p") && ffStrbufStartsWithS(&disk->mountpoint, "/mnt/"))
            {
                ffStrbufSetF(&mountpointLink, "\e]8;;file:///%c:/\e\\%s\e]8;;\e\\", disk->mountpoint.chars[5], disk->mountpoint.chars);
                ffStrbufSetF(&nameLink, "\e]8;;file:///%c:/\e\\%s\e]8;;\e\\", disk->mountpoint.chars[5], disk->name.chars);
            }
            else
            {
                ffStrbufSetF(&mountpointLink, "\e]8;;file:////wsl.localhost/%s%s\e\\%s\e]8;;\e\\", getenv("WSL_DISTRO_NAME"), disk->mountpoint.chars, disk->mountpoint.chars);
                ffStrbufSetF(&nameLink, "\e]8;;file:////wsl.localhost/%s%s\e\\%s\e]8;;\e\\", getenv("WSL_DISTRO_NAME"), disk->mountpoint.chars, disk->name.chars);
            }
        }
        else
        #endif
        {
            ffStrbufSetF(&mountpointLink, "\e]8;;file://%s\e\\%s\e]8;;\e\\", disk->mountpoint.chars, disk->mountpoint.chars);
            ffStrbufSetF(&nameLink, "\e]8;;file://%s\e\\%s\e]8;;\e\\", disk->mountpoint.chars, disk->name.chars);
        }

        FF_PARSE_FORMAT_STRING_CHECKED(&key, &options->moduleArgs.key, ((FFformatarg[]) {
            FF_FORMAT_ARG(disk->mountpoint, "mountpoint"),
            FF_FORMAT_ARG(disk->name, "name"),
            FF_FORMAT_ARG(disk->mountFrom, "mount-from"),
            FF_FORMAT_ARG(options->moduleArgs.keyIcon, "icon"),
            FF_FORMAT_ARG(index, "index"),
            FF_FORMAT_ARG(disk->filesystem, "filesystem"),
            FF_FORMAT_ARG(mountpointLink, "mountpoint-link"),
            FF_FORMAT_ARG(nameLink, "name-link"),
        }));
    }

    FF_STRBUF_AUTO_DESTROY usedPretty = ffStrbufCreate();
    ffSizeAppendNum(disk->bytesUsed, &usedPretty);

    FF_STRBUF_AUTO_DESTROY totalPretty = ffStrbufCreate();
    ffSizeAppendNum(disk->bytesTotal, &totalPretty);

    double bytesPercentage = disk->bytesTotal > 0 ? (double) disk->bytesUsed / (double) disk->bytesTotal * 100.0 : 0;
    FFPercentageTypeFlags percentType = options->percent.type == 0 ? instance.config.display.percentType : options->percent.type;

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(key.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY);

        FF_STRBUF_AUTO_DESTROY str = ffStrbufCreate();

        if(disk->bytesTotal > 0)
        {
            if(percentType & FF_PERCENTAGE_TYPE_BAR_BIT)
            {
                ffPercentAppendBar(&str, bytesPercentage, options->percent, &options->moduleArgs);
                ffStrbufAppendC(&str, ' ');
            }

            if(!(percentType & FF_PERCENTAGE_TYPE_HIDE_OTHERS_BIT))
                ffStrbufAppendF(&str, "%s / %s ", usedPretty.chars, totalPretty.chars);

            if(percentType & FF_PERCENTAGE_TYPE_NUM_BIT)
            {
                ffPercentAppendNum(&str, bytesPercentage, options->percent, str.length > 0, &options->moduleArgs);
                ffStrbufAppendC(&str, ' ');
            }
        }
        else
            ffStrbufAppendS(&str, "Unknown ");

        if(!(percentType & FF_PERCENTAGE_TYPE_HIDE_OTHERS_BIT))
        {
            if(disk->filesystem.length)
                ffStrbufAppendF(&str, "- %s ", disk->filesystem.chars);

            ffStrbufAppendC(&str, '[');
            if(disk->type & FF_DISK_VOLUME_TYPE_EXTERNAL_BIT)
                ffStrbufAppendS(&str, "External, ");
            if(disk->type & FF_DISK_VOLUME_TYPE_SUBVOLUME_BIT)
                ffStrbufAppendS(&str, "Subvolume, ");
            if(disk->type & FF_DISK_VOLUME_TYPE_HIDDEN_BIT)
                ffStrbufAppendS(&str, "Hidden, ");
            if(disk->type & FF_DISK_VOLUME_TYPE_READONLY_BIT)
                ffStrbufAppendS(&str, "Read-only, ");
            if (str.chars[str.length - 1] == '[')
                ffStrbufSubstrBefore(&str, str.length - 1);
            else
            {
                ffStrbufTrimRight(&str, ' ');
                str.chars[str.length - 1] = ']';
            }
        }

        ffStrbufTrimRight(&str, ' ');
        ffStrbufPutTo(&str, stdout);
    }
    else
    {
        FF_STRBUF_AUTO_DESTROY bytesPercentageNum = ffStrbufCreate();
        if (percentType & FF_PERCENTAGE_TYPE_NUM_BIT)
            ffPercentAppendNum(&bytesPercentageNum, bytesPercentage, options->percent, false, &options->moduleArgs);
        FF_STRBUF_AUTO_DESTROY bytesPercentageBar = ffStrbufCreate();
        if (percentType & FF_PERCENTAGE_TYPE_BAR_BIT)
            ffPercentAppendBar(&bytesPercentageBar, bytesPercentage, options->percent, &options->moduleArgs);

        double filesPercentage = disk->filesTotal > 0 ? ((double) disk->filesUsed / (double) disk->filesTotal) * 100.0 : 0;
        FF_STRBUF_AUTO_DESTROY filesPercentageNum = ffStrbufCreate();
        if (percentType & FF_PERCENTAGE_TYPE_NUM_BIT)
            ffPercentAppendNum(&filesPercentageNum, filesPercentage, options->percent, false, &options->moduleArgs);
        FF_STRBUF_AUTO_DESTROY filesPercentageBar = ffStrbufCreate();
        if (percentType & FF_PERCENTAGE_TYPE_BAR_BIT)
            ffPercentAppendBar(&filesPercentageBar, filesPercentage, options->percent, &options->moduleArgs);

        bool isExternal = !!(disk->type & FF_DISK_VOLUME_TYPE_EXTERNAL_BIT);
        bool isHidden = !!(disk->type & FF_DISK_VOLUME_TYPE_HIDDEN_BIT);
        bool isReadOnly = !!(disk->type & FF_DISK_VOLUME_TYPE_READONLY_BIT);

        FF_STRBUF_AUTO_DESTROY freePretty = ffStrbufCreate();
        ffSizeAppendNum(disk->bytesFree, &freePretty);

        FF_STRBUF_AUTO_DESTROY availPretty = ffStrbufCreate();
        ffSizeAppendNum(disk->bytesAvailable, &availPretty);

        uint64_t now = ffTimeGetNow();
        uint64_t duration = now - disk->createTime;
        uint32_t milliseconds = (uint32_t) (duration % 1000);
        duration /= 1000;
        uint32_t seconds = (uint32_t) (duration % 60);
        duration /= 60;
        uint32_t minutes = (uint32_t) (duration % 60);
        duration /= 60;
        uint32_t hours = (uint32_t) (duration % 24);
        duration /= 24;
        uint32_t days = (uint32_t) duration;

        FFTimeGetAgeResult age = ffTimeGetAge(disk->createTime, now);
        FF_PRINT_FORMAT_CHECKED(key.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY, ((FFformatarg[]) {
            FF_FORMAT_ARG(usedPretty, "size-used"),
            FF_FORMAT_ARG(totalPretty, "size-total"),
            FF_FORMAT_ARG(bytesPercentageNum, "size-percentage"),
            FF_FORMAT_ARG(disk->filesUsed, "files-used"),
            FF_FORMAT_ARG(disk->filesTotal, "files-total"),
            FF_FORMAT_ARG(filesPercentageNum, "files-percentage"),
            FF_FORMAT_ARG(isExternal, "is-external"),
            FF_FORMAT_ARG(isHidden, "is-hidden"),
            FF_FORMAT_ARG(disk->filesystem, "filesystem"),
            FF_FORMAT_ARG(disk->name, "name"),
            FF_FORMAT_ARG(isReadOnly, "is-readonly"),
            {FF_FORMAT_ARG_TYPE_STRING, ffTimeToShortStr(disk->createTime), "create-time"},
            FF_FORMAT_ARG(bytesPercentageBar, "size-percentage-bar"),
            FF_FORMAT_ARG(filesPercentageBar, "files-percentage-bar"),
            FF_FORMAT_ARG(days, "days"),
            FF_FORMAT_ARG(hours, "hours"),
            FF_FORMAT_ARG(minutes, "minutes"),
            FF_FORMAT_ARG(seconds, "seconds"),
            FF_FORMAT_ARG(milliseconds, "milliseconds"),
            FF_FORMAT_ARG(disk->mountpoint, "mountpoint"),
            FF_FORMAT_ARG(disk->mountFrom, "mount-from"),
            FF_FORMAT_ARG(age.years, "years"),
            FF_FORMAT_ARG(age.daysOfYear, "days-of-year"),
            FF_FORMAT_ARG(age.yearsFraction, "years-fraction"),
            FF_FORMAT_ARG(freePretty, "size-free"),
            FF_FORMAT_ARG(availPretty, "size-available"),
        }));
    }
}

bool ffPrintDisk(FFDiskOptions* options)
{
    FF_LIST_AUTO_DESTROY disks = ffListCreate(sizeof (FFDisk));
    const char* error = ffDetectDisks(options, &disks);

    if(error)
    {
        ffPrintError(FF_DISK_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return false;
    }

    uint32_t index = 0;
    FF_LIST_FOR_EACH(FFDisk, disk, disks)
    {
        if(__builtin_expect(options->folders.length == 0, 1) && (disk->type & ~options->showTypes))
            continue;

        if (options->hideFolders.length && ffStrbufSeparatedContain(&options->hideFolders, &disk->mountpoint, FF_DISK_FOLDER_SEPARATOR))
            continue;

        if (options->hideFS.length && ffStrbufSeparatedContain(&options->hideFS, &disk->filesystem, ':'))
            continue;

        printDisk(options, disk, ++index);
    }

    FF_LIST_FOR_EACH(FFDisk, disk, disks)
    {
        ffStrbufDestroy(&disk->mountFrom);
        ffStrbufDestroy(&disk->mountpoint);
        ffStrbufDestroy(&disk->filesystem);
        ffStrbufDestroy(&disk->name);
    }

    return true;
}

static bool setSeparatedList(FFstrbuf* strbuf, yyjson_val* val, char separator)
{
    if (yyjson_is_str(val))
    {
        ffStrbufSetJsonVal(strbuf, val);
        return true;
    }
    if (yyjson_is_arr(val))
    {
        ffStrbufClear(strbuf);
        yyjson_val *elem;
        size_t eidx, emax;
        yyjson_arr_foreach(val, eidx, emax, elem)
        {
            if (yyjson_is_str(elem))
            {
                if (strbuf->length > 0)
                    ffStrbufAppendC(strbuf, separator);
                ffStrbufAppendJsonVal(strbuf, elem);
            }
        }
        return true;
    }
    return false;
}

void ffParseDiskJsonObject(FFDiskOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        if (unsafe_yyjson_equals_str(key, "folders"))
        {
            setSeparatedList(&options->folders, val, FF_DISK_FOLDER_SEPARATOR);
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "hideFolders"))
        {
            setSeparatedList(&options->hideFolders, val, FF_DISK_FOLDER_SEPARATOR);
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "hideFS"))
        {
            setSeparatedList(&options->hideFS, val, ':');
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "showRegular"))
        {
            if (yyjson_get_bool(val))
                options->showTypes |= FF_DISK_VOLUME_TYPE_REGULAR_BIT;
            else
                options->showTypes &= ~FF_DISK_VOLUME_TYPE_REGULAR_BIT;
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "showExternal"))
        {
            if (yyjson_get_bool(val))
                options->showTypes |= FF_DISK_VOLUME_TYPE_EXTERNAL_BIT;
            else
                options->showTypes &= ~FF_DISK_VOLUME_TYPE_EXTERNAL_BIT;
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "showHidden"))
        {
            if (yyjson_get_bool(val))
                options->showTypes |= FF_DISK_VOLUME_TYPE_HIDDEN_BIT;
            else
                options->showTypes &= ~FF_DISK_VOLUME_TYPE_HIDDEN_BIT;
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "showSubvolumes"))
        {
            if (yyjson_get_bool(val))
                options->showTypes |= FF_DISK_VOLUME_TYPE_SUBVOLUME_BIT;
            else
                options->showTypes &= ~FF_DISK_VOLUME_TYPE_SUBVOLUME_BIT;
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "showReadOnly"))
        {
            if (yyjson_get_bool(val))
                options->showTypes |= FF_DISK_VOLUME_TYPE_READONLY_BIT;
            else
                options->showTypes &= ~FF_DISK_VOLUME_TYPE_READONLY_BIT;
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "showUnknown"))
        {
            if (yyjson_get_bool(val))
                options->showTypes |= FF_DISK_VOLUME_TYPE_UNKNOWN_BIT;
            else
                options->showTypes &= ~FF_DISK_VOLUME_TYPE_UNKNOWN_BIT;
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "useAvailable"))
        {
            if (yyjson_get_bool(val))
                options->calcType = FF_DISK_CALC_TYPE_AVAILABLE;
            else
                options->calcType = FF_DISK_CALC_TYPE_FREE;
            continue;
        }

        if (ffPercentParseJsonObject(key, val, &options->percent))
            continue;

        ffPrintError(FF_DISK_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateDiskJsonConfig(FFDiskOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);

    yyjson_mut_obj_add_bool(doc, module, "showRegular", !!(options->showTypes & FF_DISK_VOLUME_TYPE_REGULAR_BIT));

    yyjson_mut_obj_add_bool(doc, module, "showExternal", !!(options->showTypes & FF_DISK_VOLUME_TYPE_EXTERNAL_BIT));

    yyjson_mut_obj_add_bool(doc, module, "showHidden", !!(options->showTypes & FF_DISK_VOLUME_TYPE_HIDDEN_BIT));

    yyjson_mut_obj_add_bool(doc, module, "showSubvolumes", !!(options->showTypes & FF_DISK_VOLUME_TYPE_SUBVOLUME_BIT));

    yyjson_mut_obj_add_bool(doc, module, "showReadOnly", !!(options->showTypes & FF_DISK_VOLUME_TYPE_READONLY_BIT));

    yyjson_mut_obj_add_bool(doc, module, "showUnknown", !!(options->showTypes & FF_DISK_VOLUME_TYPE_UNKNOWN_BIT));

    yyjson_mut_obj_add_strbuf(doc, module, "folders", &options->folders);

    yyjson_mut_obj_add_strbuf(doc, module, "hideFolders", &options->hideFolders);

    yyjson_mut_obj_add_strbuf(doc, module, "hideFS", &options->hideFS);

    yyjson_mut_obj_add_bool(doc, module, "useAvailable", options->calcType == FF_DISK_CALC_TYPE_AVAILABLE);

    ffPercentGenerateJsonConfig(doc, module, options->percent);
}

bool ffGenerateDiskJsonResult(FFDiskOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FF_LIST_AUTO_DESTROY disks = ffListCreate(sizeof (FFDisk));
    const char* error = ffDetectDisks(options, &disks);

    if(error)
    {
        yyjson_mut_obj_add_str(doc, module, "result", error);
        return false;
    }

    yyjson_mut_val* arr = yyjson_mut_obj_add_arr(doc, module, "result");

    FF_LIST_FOR_EACH(FFDisk, item, disks)
    {
        yyjson_mut_val* obj = yyjson_mut_arr_add_obj(doc, arr);

        yyjson_mut_val* bytes = yyjson_mut_obj_add_obj(doc, obj, "bytes");
        yyjson_mut_obj_add_uint(doc, bytes, "available", item->bytesAvailable);
        yyjson_mut_obj_add_uint(doc, bytes, "free", item->bytesFree);
        yyjson_mut_obj_add_uint(doc, bytes, "total", item->bytesTotal);
        yyjson_mut_obj_add_uint(doc, bytes, "used", item->bytesUsed);

        yyjson_mut_val* files = yyjson_mut_obj_add_obj(doc, obj, "files");
        if (item->filesTotal == 0 && item->filesUsed == 0)
        {
            yyjson_mut_obj_add_null(doc, files, "total");
            yyjson_mut_obj_add_null(doc, files, "used");
        }
        else
        {
            yyjson_mut_obj_add_uint(doc, files, "total", item->filesTotal);
            yyjson_mut_obj_add_uint(doc, files, "used", item->filesUsed);
        }

        yyjson_mut_obj_add_strbuf(doc, obj, "filesystem", &item->filesystem);
        yyjson_mut_obj_add_strbuf(doc, obj, "mountpoint", &item->mountpoint);
        yyjson_mut_obj_add_strbuf(doc, obj, "mountFrom", &item->mountFrom);
        yyjson_mut_obj_add_strbuf(doc, obj, "name", &item->name);
        yyjson_mut_val* typeArr = yyjson_mut_obj_add_arr(doc, obj, "volumeType");
        if(item->type & FF_DISK_VOLUME_TYPE_REGULAR_BIT)
            yyjson_mut_arr_add_str(doc, typeArr, "Regular");
        if(item->type & FF_DISK_VOLUME_TYPE_EXTERNAL_BIT)
            yyjson_mut_arr_add_str(doc, typeArr, "External");
        if(item->type & FF_DISK_VOLUME_TYPE_SUBVOLUME_BIT)
            yyjson_mut_arr_add_str(doc, typeArr, "Subvolume");
        if(item->type & FF_DISK_VOLUME_TYPE_HIDDEN_BIT)
            yyjson_mut_arr_add_str(doc, typeArr, "Hidden");
        if(item->type & FF_DISK_VOLUME_TYPE_READONLY_BIT)
            yyjson_mut_arr_add_str(doc, typeArr, "Read-only");
        if(item->type & FF_DISK_VOLUME_TYPE_UNKNOWN_BIT)
            yyjson_mut_arr_add_str(doc, typeArr, "Unknown");

        const char* pstr = ffTimeToFullStr(item->createTime);
        if (*pstr)
            yyjson_mut_obj_add_strcpy(doc, obj, "createTime", pstr);
        else
            yyjson_mut_obj_add_null(doc, obj, "createTime");
    }

    FF_LIST_FOR_EACH(FFDisk, item, disks)
    {
        ffStrbufDestroy(&item->mountpoint);
        ffStrbufDestroy(&item->mountFrom);
        ffStrbufDestroy(&item->filesystem);
        ffStrbufDestroy(&item->name);
    }

    return true;
}

void ffInitDiskOptions(FFDiskOptions* options)
{
    ffOptionInitModuleArg(&options->moduleArgs, "");

    ffStrbufInit(&options->folders);
    #if _WIN32 || __APPLE__ || __ANDROID__
    ffStrbufInit(&options->hideFolders);
    #else
    ffStrbufInitStatic(&options->hideFolders, "/efi:/boot:/boot/efi:/boot/firmware");
    #endif
    ffStrbufInit(&options->hideFS);
    options->showTypes = FF_DISK_VOLUME_TYPE_REGULAR_BIT | FF_DISK_VOLUME_TYPE_EXTERNAL_BIT | FF_DISK_VOLUME_TYPE_READONLY_BIT;
    options->calcType = FF_DISK_CALC_TYPE_FREE;
    options->percent = (FFPercentageModuleConfig) { 50, 80, 0 };
}

void ffDestroyDiskOptions(FFDiskOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
    ffStrbufDestroy(&options->folders);
    ffStrbufDestroy(&options->hideFolders);
    ffStrbufDestroy(&options->hideFS);
}

FFModuleBaseInfo ffDiskModuleInfo = {
    .name = FF_DISK_MODULE_NAME,
    .description = "Print partitions, space usage, file system, etc",
    .initOptions = (void*) ffInitDiskOptions,
    .destroyOptions = (void*) ffDestroyDiskOptions,
    .parseJsonObject = (void*) ffParseDiskJsonObject,
    .printModule = (void*) ffPrintDisk,
    .generateJsonResult = (void*) ffGenerateDiskJsonResult,
    .generateJsonConfig = (void*) ffGenerateDiskJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"Size used", "size-used"},
        {"Size total", "size-total"},
        {"Size percentage num", "size-percentage"},
        {"Files used", "files-used"},
        {"Files total", "files-total"},
        {"Files percentage num", "files-percentage"},
        {"True if external volume", "is-external"},
        {"True if hidden volume", "is-hidden"},
        {"Filesystem", "filesystem"},
        {"Label / name", "name"},
        {"True if read-only", "is-readonly"},
        {"Create time in local timezone", "create-time"},
        {"Size percentage bar", "size-percentage-bar"},
        {"Files percentage bar", "files-percentage-bar"},
        {"Days after creation", "days"},
        {"Hours after creation", "hours"},
        {"Minutes after creation", "minutes"},
        {"Seconds after creation", "seconds"},
        {"Milliseconds after creation", "milliseconds"},
        {"Mount point / drive letter", "mountpoint"},
        {"Mount from (device path)", "mount-from"},
        {"Years integer after creation", "years"},
        {"Days of year after creation", "days-of-year"},
        {"Years fraction after creation", "years-fraction"},
        {"Size free", "size-free"},
        {"Size available", "size-available"},
    }))
};
