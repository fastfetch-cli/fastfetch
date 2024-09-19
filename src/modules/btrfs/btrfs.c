#include "common/printing.h"
#include "common/jsonconfig.h"
#include "common/percent.h"
#include "detection/btrfs/btrfs.h"
#include "modules/btrfs/btrfs.h"
#include "util/stringUtils.h"

#define FF_BTRFS_NUM_FORMAT_ARGS 13

static void printBtrfs(FFBtrfsOptions* options, FFBtrfsResult* result, uint8_t index)
{
    FF_STRBUF_AUTO_DESTROY buffer = ffStrbufCreate();
    if (options->moduleArgs.key.length == 0)
    {
        if (result->name.length > 0)
            ffStrbufSetF(&buffer, "%s (%s)", FF_BTRFS_MODULE_NAME, result->name.chars);
        else
            ffStrbufSetS(&buffer, FF_BTRFS_MODULE_NAME);
    }
    else
    {
        ffStrbufClear(&buffer);
        FF_PARSE_FORMAT_STRING_CHECKED(&buffer, &options->moduleArgs.key, 3, ((FFformatarg[]){
            FF_FORMAT_ARG(index, "index"),
            FF_FORMAT_ARG(result->name, "name"),
            FF_FORMAT_ARG(options->moduleArgs.keyIcon, "icon"),
        }));
    }

    uint64_t used = 0, allocated = 0, total = result->totalSize;
    for (int i = 0; i < 3; ++i)
    {
        uint64_t times = result->allocation[i].dup ? 2 : 1;
        used += result->allocation[i].used * times;
        allocated += result->allocation[i].total * times;
    }

    FF_STRBUF_AUTO_DESTROY usedPretty = ffStrbufCreate();
    ffParseSize(used, &usedPretty);
    FF_STRBUF_AUTO_DESTROY allocatedPretty = ffStrbufCreate();
    ffParseSize(allocated, &allocatedPretty);
    FF_STRBUF_AUTO_DESTROY totalPretty = ffStrbufCreate();
    ffParseSize(total, &totalPretty);

    double usedPercentage = total > 0 ? (double) used / (double) total * 100.0 : 0;
    double allocatedPercentage = total > 0 ? (double) allocated / (double) total * 100.0 : 0;

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(buffer.chars, index, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY);

        ffStrbufClear(&buffer);
        ffStrbufSetF(&buffer, "%s / %s (", usedPretty.chars, totalPretty.chars);
        ffPercentAppendNum(&buffer, usedPercentage, options->percent, false, &options->moduleArgs);
        ffStrbufAppendS(&buffer, ", ");
        ffPercentAppendNum(&buffer, allocatedPercentage, options->percent, false, &options->moduleArgs);
        ffStrbufAppendF(&buffer, " allocated)");
        ffStrbufPutTo(&buffer, stdout);
    }
    else
    {
        FF_STRBUF_AUTO_DESTROY usedPercentageNum = ffStrbufCreate();
        ffPercentAppendNum(&usedPercentageNum, usedPercentage, options->percent, false, &options->moduleArgs);
        FF_STRBUF_AUTO_DESTROY usedPercentageBar = ffStrbufCreate();
        ffPercentAppendBar(&usedPercentageBar, usedPercentage, options->percent, &options->moduleArgs);

        FF_STRBUF_AUTO_DESTROY allocatedPercentageNum = ffStrbufCreate();
        ffPercentAppendNum(&allocatedPercentageNum, allocatedPercentage, options->percent, false, &options->moduleArgs);
        FF_STRBUF_AUTO_DESTROY allocatedPercentageBar = ffStrbufCreate();
        ffPercentAppendBar(&allocatedPercentageBar, allocatedPercentage, options->percent, &options->moduleArgs);

        FF_STRBUF_AUTO_DESTROY nodeSizePretty = ffStrbufCreate();
        ffParseSize(result->nodeSize, &nodeSizePretty);
        FF_STRBUF_AUTO_DESTROY sectorSizePretty = ffStrbufCreate();
        ffParseSize(result->sectorSize, &sectorSizePretty);

        FF_PRINT_FORMAT_CHECKED(buffer.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY, FF_BTRFS_NUM_FORMAT_ARGS, ((FFformatarg[]) {
            FF_FORMAT_ARG(result->name, "name"),
            FF_FORMAT_ARG(result->uuid, "uuid"),
            FF_FORMAT_ARG(result->devices, "devices"),
            FF_FORMAT_ARG(result->features, "features"),
            FF_FORMAT_ARG(usedPretty, "used"),
            FF_FORMAT_ARG(allocatedPretty, "allocated"),
            FF_FORMAT_ARG(totalPretty, "total"),
            FF_FORMAT_ARG(usedPercentageNum, "used-percentage"),
            FF_FORMAT_ARG(allocatedPercentageNum, "allocated-percentage"),
            FF_FORMAT_ARG(usedPercentageBar, "used-percentage-bar"),
            FF_FORMAT_ARG(allocatedPercentageBar, "allocated-percentage-bar"),
            FF_FORMAT_ARG(nodeSizePretty, "node-size"),
            FF_FORMAT_ARG(sectorSizePretty, "sector-size"),
        }));
    }
}

void ffPrintBtrfs(FFBtrfsOptions* options)
{
    FF_LIST_AUTO_DESTROY results = ffListCreate(sizeof(FFBtrfsResult));

    const char* error = ffDetectBtrfs(&results);

    if (error)
    {
        ffPrintError(FF_BTRFS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return;
    }
    if(results.length == 0)
    {
        ffPrintError(FF_BTRFS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", "No btrfs drive found");
        return;
    }

    for(uint32_t i = 0; i < results.length; i++)
    {
        FFBtrfsResult* result = FF_LIST_GET(FFBtrfsResult, results, i);
        uint8_t index = results.length == 1 ? 0 : (uint8_t) (i + 1);
        printBtrfs(options, result, index);
    }

    FF_LIST_FOR_EACH(FFBtrfsResult, result, results)
    {
        ffStrbufDestroy(&result->name);
        ffStrbufDestroy(&result->uuid);
        ffStrbufDestroy(&result->devices);
        ffStrbufDestroy(&result->features);
    }
}

bool ffParseBtrfsCommandOptions(FFBtrfsOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_BTRFS_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    if (ffPercentParseCommandOptions(key, subKey, value, &options->percent))
        return true;

    return false;
}

void ffParseBtrfsJsonObject(FFBtrfsOptions* options, yyjson_val* module)
{
    yyjson_val *key_, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key_, val)
    {
        const char* key = yyjson_get_str(key_);
        if(ffStrEqualsIgnCase(key, "type"))
            continue;

        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        if (ffPercentParseJsonObject(key, val, &options->percent))
            continue;

        ffPrintError(FF_BTRFS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", key);
    }
}

void ffGenerateBtrfsJsonConfig(FFBtrfsOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    __attribute__((__cleanup__(ffDestroyBtrfsOptions))) FFBtrfsOptions defaultOptions;
    ffInitBtrfsOptions(&defaultOptions);

    ffJsonConfigGenerateModuleArgsConfig(doc, module, &defaultOptions.moduleArgs, &options->moduleArgs);

    ffPercentGenerateJsonConfig(doc, module, defaultOptions.percent, options->percent);
}

void ffGenerateBtrfsJsonResult(FF_MAYBE_UNUSED FFBtrfsOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FF_LIST_AUTO_DESTROY results = ffListCreate(sizeof(FFBtrfsResult));

    const char* error = ffDetectBtrfs(&results);
    if (error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return;
    }

    yyjson_mut_val* arr = yyjson_mut_obj_add_arr(doc, module, "result");

    FF_LIST_FOR_EACH(FFBtrfsResult, btrfs, results)
    {
        yyjson_mut_val* obj = yyjson_mut_arr_add_obj(doc, arr);
        yyjson_mut_obj_add_strbuf(doc, obj, "name", &btrfs->name);
        yyjson_mut_obj_add_strbuf(doc, obj, "uuid", &btrfs->uuid);
        yyjson_mut_obj_add_strbuf(doc, obj, "devices", &btrfs->devices);
        yyjson_mut_obj_add_strbuf(doc, obj, "features", &btrfs->features);
        yyjson_mut_obj_add_uint(doc, obj, "generation", btrfs->generation);
        yyjson_mut_obj_add_uint(doc, obj, "nodeSize", btrfs->nodeSize);
        yyjson_mut_obj_add_uint(doc, obj, "sectorSize", btrfs->sectorSize);
        yyjson_mut_obj_add_uint(doc, obj, "totalSize", btrfs->totalSize);
        yyjson_mut_val* allocation = yyjson_mut_obj_add_arr(doc, obj, "allocation");
        for (int i = 0; i < 3; ++i)
        {
            yyjson_mut_val* item = yyjson_mut_arr_add_obj(doc, allocation);
            yyjson_mut_obj_add_str(doc, item, "type", btrfs->allocation[i].type);
            yyjson_mut_obj_add_bool(doc, item, "dup", btrfs->allocation[i].dup);
            yyjson_mut_obj_add_uint(doc, item, "used", btrfs->allocation[i].used);
            yyjson_mut_obj_add_uint(doc, item, "total", btrfs->allocation[i].total);
        }
    }

    FF_LIST_FOR_EACH(FFBtrfsResult, btrfs, results)
    {
        ffStrbufDestroy(&btrfs->name);
        ffStrbufDestroy(&btrfs->uuid);
        ffStrbufDestroy(&btrfs->devices);
        ffStrbufDestroy(&btrfs->features);
    }
}

void ffPrintBtrfsHelpFormat(void)
{
    FF_PRINT_MODULE_FORMAT_HELP_CHECKED(FF_BTRFS_MODULE_NAME, "{5} / {7} ({8}, {9} allocated)", FF_BTRFS_NUM_FORMAT_ARGS, ((const char* []) {
        "Name / Label - name",
        "UUID - uuid",
        "Associated devices - devices",
        "Enabled features - features",
        "Size used - used",
        "Size allocated - allocated",
        "Size total - total",
        "Used percentage num - used-percentage",
        "Allocated percentage num - allocated-percentage",
        "Used percentage bar - used-percentage-bar",
        "Allocated percentage bar - allocated-percentage-bar",
        "Node size - node-size",
        "Sector size - sector-size",
    }));
}

void ffInitBtrfsOptions(FFBtrfsOptions* options)
{
    ffOptionInitModuleBaseInfo(
        &options->moduleInfo,
        FF_BTRFS_MODULE_NAME,
        "Print BTRFS volumes",
        ffParseBtrfsCommandOptions,
        ffParseBtrfsJsonObject,
        ffPrintBtrfs,
        ffGenerateBtrfsJsonResult,
        ffPrintBtrfsHelpFormat,
        ffGenerateBtrfsJsonConfig
    );
    ffOptionInitModuleArg(&options->moduleArgs, "󱑛");
    options->percent = (FFColorRangeConfig) { 50, 80 };
}

void ffDestroyBtrfsOptions(FFBtrfsOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}
