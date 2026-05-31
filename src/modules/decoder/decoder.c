#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/decoder/decoder.h"
#include "modules/decoder/decoder.h"

static const char* ffDecoderTypeToString(FFDecoderType type) {
    switch (type) {
        case FF_DECODER_TYPE_H261:
            return "H.261";
        case FF_DECODER_TYPE_H263:
            return "H.263";
        case FF_DECODER_TYPE_MJPEG:
            return "MJPEG";
        case FF_DECODER_TYPE_MPEG1:
            return "MPEG-1";
        case FF_DECODER_TYPE_MPEG2:
            return "MPEG-2";
        case FF_DECODER_TYPE_DIVX_XVID:
            return "DivX / Xvid";
        case FF_DECODER_TYPE_H264:
            return "H.264";
        case FF_DECODER_TYPE_WMV8:
            return "WMV-8";
        case FF_DECODER_TYPE_WMV9:
            return "WMV-9";
        case FF_DECODER_TYPE_VC1:
            return "VC-1";
        case FF_DECODER_TYPE_VP8:
            return "VP8";
        case FF_DECODER_TYPE_HEVC:
            return "HEVC / H.265";
        case FF_DECODER_TYPE_VP9:
            return "VP9";
        case FF_DECODER_TYPE_AV1:
            return "AV1";
        case FF_DECODER_TYPE_VVC:
            return "VVC / H.266";
        case FF_DECODER_TYPE_UNKNOWN:
        default:
            return "Unknown";
    }
}

static void ffDestroyDecoderResults(FFlist* result) {
    FF_LIST_FOR_EACH (FFDecoderResult, item, *result) {
        ffStrbufDestroy(&item->gpu);
    }
}

static void printDecoder(const FFDecoderOptions* options, const FFDecoderResult* decoder, uint8_t index) {
    FF_STRBUF_AUTO_DESTROY key = ffStrbufCreate();

    if (options->moduleArgs.key.length == 0) {
        ffStrbufSetF(&key, "%s (%s)", FF_DECODER_MODULE_NAME, decoder->gpu.chars);
    } else {
        FF_PARSE_FORMAT_STRING_CHECKED(&key, &options->moduleArgs.key, ((FFformatarg[]){
                                                                           FF_ARG(decoder->gpu, "gpu"),
                                                                           FF_ARG(options->moduleArgs.keyIcon, "icon"),
                                                                           FF_ARG(index, "index"),
                                                                       }));
    }

    if (options->moduleArgs.outputFormat.length == 0) {
        FF_STRBUF_AUTO_DESTROY typesJoined = ffStrbufCreate();
        for (FFDecoderType type = FF_DECODER_TYPE_UNKNOWN; type <= FF_DECODER_TYPE_MAX; type <<= 1) {
            if ((decoder->types & type) == 0) {
                continue;
            }
            if (typesJoined.length > 0) {
                ffStrbufAppendS(&typesJoined, ", ");
            }
            ffStrbufAppendS(&typesJoined, ffDecoderTypeToString(type));
        }

        ffPrintLogoAndKey(key.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY);

        if (typesJoined.length > 0) {
            ffStrbufWriteTo(&typesJoined, stdout);
        }
        putchar('\n');
    } else {
        FF_LIST_AUTO_DESTROY types = ffListCreate(); // Use list instead of pre-joined string for qjs and lua
        for (FFDecoderType type = FF_DECODER_TYPE_UNKNOWN; type <= FF_DECODER_TYPE_MAX; type <<= 1) {
            if ((decoder->types & type) == 0) {
                continue;
            }
            ffStrbufInitStatic(FF_LIST_ADD(FFstrbuf, types), ffDecoderTypeToString(type));
        }

        FF_PRINT_FORMAT_CHECKED(key.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY, ((FFformatarg[]){
                                                                                                     FF_ARG(decoder->gpu, "gpu"),
                                                                                                     FF_ARG(types, "types"),
                                                                                                 }));
        // No need to destroy strings in types list, as they are static strings
    }
}

bool ffPrintDecoder(FFDecoderOptions* options) {
    FF_LIST_AUTO_DESTROY result = ffListCreate();
    const char* error = ffDetectDecoder(NULL, &result);

    if (error) {
        ffPrintError(FF_DECODER_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return false;
    }

    if (result.length == 0) {
        ffPrintError(FF_DECODER_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "No hardware decoder found");
        return false;
    }

    for (uint32_t i = 0; i < result.length; ++i) {
        uint8_t index = (uint8_t) (result.length == 1 ? 0 : i + 1);
        printDecoder(options, FF_LIST_GET(FFDecoderResult, result, i), index);
    }

    ffDestroyDecoderResults(&result);
    return true;
}

void ffParseDecoderJsonObject(FFDecoderOptions* options, yyjson_val* module) {
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach (module, idx, max, key, val) {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs)) {
            continue;
        }

        ffPrintError(FF_DECODER_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateDecoderJsonConfig(FFDecoderOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module) {
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);
}

bool ffGenerateDecoderJsonResult(FF_A_UNUSED FFDecoderOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module) {
    FF_LIST_AUTO_DESTROY result = ffListCreate();
    const char* error = ffDetectDecoder(NULL, &result);

    if (error) {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return false;
    }

    yyjson_mut_val* arr = yyjson_mut_obj_add_arr(doc, module, "result");

    FF_LIST_FOR_EACH (FFDecoderResult, item, result) {
        yyjson_mut_val* obj = yyjson_mut_arr_add_obj(doc, arr);
        yyjson_mut_obj_add_strbuf(doc, obj, "gpu", &item->gpu);

        yyjson_mut_val* types = yyjson_mut_obj_add_arr(doc, obj, "types");
        for (FFDecoderType type = FF_DECODER_TYPE_UNKNOWN; type <= FF_DECODER_TYPE_MAX; type <<= 1) {
            if ((item->types & type) == 0) {
                continue;
            }
            yyjson_mut_arr_add_str(doc, types, ffDecoderTypeToString(type));
        }
    }

    ffDestroyDecoderResults(&result);
    return true;
}

void ffInitDecoderOptions(FFDecoderOptions* options) {
    ffOptionInitModuleArg(&options->moduleArgs, "󰚔");
}

void ffDestroyDecoderOptions(FFDecoderOptions* options) {
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffDecoderModuleInfo = {
    .name = FF_DECODER_MODULE_NAME,
    .description = "Print hardware decoder types grouped by GPU",
    .initOptions = (void*) ffInitDecoderOptions,
    .destroyOptions = (void*) ffDestroyDecoderOptions,
    .parseJsonObject = (void*) ffParseDecoderJsonObject,
    .printModule = (void*) ffPrintDecoder,
    .generateJsonResult = (void*) ffGenerateDecoderJsonResult,
    .generateJsonConfig = (void*) ffGenerateDecoderJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]){
        { "GPU name", "gpu" },
        { "Decoder types, comma-separated", "types" },
    }))
};
