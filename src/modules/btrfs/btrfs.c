#include "common/printing.h"
#include "common/jsonconfig.h"
#include "common/percent.h"
#include "common/size.h"
#include "detection/btrfs/btrfs.h"
#include "modules/btrfs/btrfs.h"
#include "util/stringUtils.h"

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
        FF_PARSE_FORMAT_STRING_CHECKED(&buffer, &options->moduleArgs.key, ((FFformatarg[]) {
            FF_FORMAT_ARG(index, "index"),
            FF_FORMAT_ARG(result->name, "name"),
            FF_FORMAT_ARG(options->moduleArgs.keyIcon, "icon"),
        }));
    }

    uint64_t used = 0, allocated = 0, total = result->totalSize;
    for (uint32_t i = 0; i < ARRAY_SIZE(result->allocation); ++i)
    {
        uint64_t times = result->allocation[i].copies;
        used += result->allocation[i].used * times;
        allocated += result->allocation[i].total * times;
    }

    FF_STRBUF_AUTO_DESTROY usedPretty = ffStrbufCreate();
    ffSizeAppendNum(used, &usedPretty);
    FF_STRBUF_AUTO_DESTROY allocatedPretty = ffStrbufCreate();
    ffSizeAppendNum(allocated, &allocatedPretty);
    FF_STRBUF_AUTO_DESTROY totalPretty = ffStrbufCreate();
    ffSizeAppendNum(total, &totalPretty);

    double usedPercentage = total > 0 ? (double) used / (double) total * 100.0 : 0;
    double allocatedPercentage = total > 0 ? (double) allocated / (double) total * 100.0 : 0;

    FFPercentageTypeFlags percentType = options->percent.type == 0 ? instance.config.display.percentType : options->percent.type;

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
        if (percentType & FF_PERCENTAGE_TYPE_NUM_BIT)
            ffPercentAppendNum(&usedPercentageNum, usedPercentage, options->percent, false, &options->moduleArgs);
        FF_STRBUF_AUTO_DESTROY usedPercentageBar = ffStrbufCreate();
        if (percentType & FF_PERCENTAGE_TYPE_BAR_BIT)
            ffPercentAppendBar(&usedPercentageBar, usedPercentage, options->percent, &options->moduleArgs);

        FF_STRBUF_AUTO_DESTROY allocatedPercentageNum = ffStrbufCreate();
        if (percentType & FF_PERCENTAGE_TYPE_NUM_BIT)
            ffPercentAppendNum(&allocatedPercentageNum, allocatedPercentage, options->percent, false, &options->moduleArgs);
        FF_STRBUF_AUTO_DESTROY allocatedPercentageBar = ffStrbufCreate();
        if (percentType & FF_PERCENTAGE_TYPE_BAR_BIT)
            ffPercentAppendBar(&allocatedPercentageBar, allocatedPercentage, options->percent, &options->moduleArgs);

        FF_STRBUF_AUTO_DESTROY nodeSizePretty = ffStrbufCreate();
        ffSizeAppendNum(result->nodeSize, &nodeSizePretty);
        FF_STRBUF_AUTO_DESTROY sectorSizePretty = ffStrbufCreate();
        ffSizeAppendNum(result->sectorSize, &sectorSizePretty);

        FF_PRINT_FORMAT_CHECKED(buffer.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY, ((FFformatarg[]) {
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

bool ffPrintBtrfs(FFBtrfsOptions* options)
{
    FF_LIST_AUTO_DESTROY results = ffListCreate(sizeof(FFBtrfsResult));

    const char* error = ffDetectBtrfs(&results);

    if (error)
    {
        ffPrintError(FF_BTRFS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return false;
    }
    if(results.length == 0)
    {
        ffPrintError(FF_BTRFS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", "No btrfs drive found");
        return false;
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

    return true;
}

void ffParseBtrfsJsonObject(FFBtrfsOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        if (ffPercentParseJsonObject(key, val, &options->percent))
            continue;

        ffPrintError(FF_BTRFS_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateBtrfsJsonConfig(FFBtrfsOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);

    ffPercentGenerateJsonConfig(doc, module, options->percent);
}

bool ffGenerateBtrfsJsonResult(FF_MAYBE_UNUSED FFBtrfsOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    FF_LIST_AUTO_DESTROY results = ffListCreate(sizeof(FFBtrfsResult));

    const char* error = ffDetectBtrfs(&results);
    if (error)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return false;
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
        for (uint32_t i = 0; i < ARRAY_SIZE(btrfs->allocation); ++i)
        {
            yyjson_mut_val* item = yyjson_mut_arr_add_obj(doc, allocation);
            yyjson_mut_obj_add_str(doc, item, "type", btrfs->allocation[i].type);
            yyjson_mut_obj_add_str(doc, item, "profile", btrfs->allocation[i].profile);
            yyjson_mut_obj_add_uint(doc, item, "copies", btrfs->allocation[i].copies);
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

    return true;
}

void ffInitBtrfsOptions(FFBtrfsOptions* options)
{
    ffOptionInitModuleArg(&options->moduleArgs, "󱑛");
    options->percent = (FFPercentageModuleConfig) { 50, 80, 0 };
}

void ffDestroyBtrfsOptions(FFBtrfsOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffBtrfsModuleInfo = {
    .name = FF_BTRFS_MODULE_NAME,
    .description = "Print Linux BTRFS volumes",
    .initOptions = (void*) ffInitBtrfsOptions,
    .destroyOptions = (void*) ffDestroyBtrfsOptions,
    .parseJsonObject = (void*) ffParseBtrfsJsonObject,
    .printModule = (void*) ffPrintBtrfs,
    .generateJsonResult = (void*) ffGenerateBtrfsJsonResult,
    .generateJsonConfig = (void*) ffGenerateBtrfsJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"Name / Label", "name"},
        {"UUID", "uuid"},
        {"Associated devices", "devices"},
        {"Enabled features", "features"},
        {"Size used", "used"},
        {"Size allocated", "allocated"},
        {"Size total", "total"},
        {"Used percentage num", "used-percentage"},
        {"Allocated percentage num", "allocated-percentage"},
        {"Used percentage bar", "used-percentage-bar"},
        {"Allocated percentage bar", "allocated-percentage-bar"},
        {"Node size", "node-size"},
        {"Sector size", "sector-size"},
    }))
};
