#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/codec/codec.h"
#include "modules/codec/codec.h"

static const char* ffCodecTypeToString(FFCodecType type) {
    switch (type) {
        case FF_CODEC_TYPE_H261:
            return "H.261";
        case FF_CODEC_TYPE_H263:
            return "H.263";
        case FF_CODEC_TYPE_MJPEG:
            return "MJPEG";
        case FF_CODEC_TYPE_MPEG1:
            return "MPEG-1";
        case FF_CODEC_TYPE_MPEG2:
            return "MPEG-2";
        case FF_CODEC_TYPE_DIVX_XVID:
            return "DivX / Xvid";
        case FF_CODEC_TYPE_H264:
            return "H.264";
        case FF_CODEC_TYPE_WMV8:
            return "WMV-8";
        case FF_CODEC_TYPE_WMV9:
            return "WMV-9";
        case FF_CODEC_TYPE_VC1:
            return "VC-1";
        case FF_CODEC_TYPE_VP8:
            return "VP8";
        case FF_CODEC_TYPE_HEVC:
            return "HEVC / H.265";
        case FF_CODEC_TYPE_VP9:
            return "VP9";
        case FF_CODEC_TYPE_AV1:
            return "AV1";
        case FF_CODEC_TYPE_VVC:
            return "VVC / H.266";
        case FF_CODEC_TYPE_DOLBY_VISION_HEVC:
            return "Dolby Vision (HEVC)";
        case FF_CODEC_TYPE_PRORES:
            return "Apple ProRes";
        case FF_CODEC_TYPE_PRORES_RAW:
            return "Apple ProRes RAW";
        case FF_CODEC_TYPE_JPEG_XL:
            return "JPEG XL";
        case FF_CODEC_TYPE_UNKNOWN:
        default:
            return "Unknown";
    }
}

static void printCodecLine(const FFCodecOptions* options, uint8_t index, FFstrbuf* gpu, const char* direction, FFCodecType types, const char* platformApi) {
    FF_STRBUF_AUTO_DESTROY key = ffStrbufCreate();
    if (options->moduleArgs.key.length == 0) {
        if (gpu->length > 0) {
            ffStrbufSetF(&key, "%s (%s - %s)", FF_CODEC_MODULE_NAME, direction, gpu->chars);
        } else {
            ffStrbufSetF(&key, "%s (%s)", FF_CODEC_MODULE_NAME, direction);
        }
    } else {
        FF_PARSE_FORMAT_STRING_CHECKED(&key, &options->moduleArgs.key, ((FFformatarg[]){
                                                                           FF_ARG(index, "index"),
                                                                           FF_ARG(*gpu, "gpu"),
                                                                           FF_ARG(direction, "direction"),
                                                                           FF_ARG(options->moduleArgs.keyIcon, "icon"),
                                                                       }));
    }

    if (options->moduleArgs.outputFormat.length == 0) {
        FF_STRBUF_AUTO_DESTROY typesJoined = ffStrbufCreate();
        for (FFCodecType type = FF_CODEC_TYPE_H261; type <= FF_CODEC_TYPE_MAX; type <<= 1) {
            if ((types & type) == 0) {
                continue;
            }
            if (typesJoined.length > 0) {
                ffStrbufAppendS(&typesJoined, ", ");
            }
            ffStrbufAppendS(&typesJoined, ffCodecTypeToString(type));
        }
        ffPrintLogoAndKey(key.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY);
        puts(typesJoined.length ? typesJoined.chars : "None");
    } else {
        FF_LIST_AUTO_DESTROY typeList = ffListCreate(); // Use list instead of pre-joined string for qjs and lua
        for (FFCodecType type = FF_CODEC_TYPE_H261; type <= FF_CODEC_TYPE_MAX; type <<= 1) {
            if ((types & type) == 0) {
                continue;
            }
            ffStrbufInitStatic(FF_LIST_ADD(FFstrbuf, typeList), ffCodecTypeToString(type));
        }
        FF_PRINT_FORMAT_CHECKED(key.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY, ((FFformatarg[]){
                                                                                                     FF_ARG(*gpu, "gpu"),
                                                                                                     FF_ARG(direction, "direction"),
                                                                                                     FF_ARG(typeList, "types"),
                                                                                                     FF_ARG(platformApi, "platform-api"),
                                                                                                 }));
        // No need to destroy strings in lists, as they are static strings
    }
}

bool ffPrintCodec(FFCodecOptions* options) {
    FF_LIST_AUTO_DESTROY result = ffListCreate();
    const char* error = ffDetectCodec(options, &result);

    if (error) {
        ffPrintError(FF_CODEC_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return false;
    }

    if (result.length == 0) {
        ffPrintError(FF_CODEC_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "No hardware video acceleration found");
        return false;
    }

    FF_STRBUF_AUTO_DESTROY key = ffStrbufCreate();
    if (options->splitGPU) {
        for (uint32_t i = 0; i < result.length; ++i) {
            FFCodecResult* item = FF_LIST_GET(FFCodecResult, result, i);
            uint8_t index = (uint8_t) (result.length == 1 ? 0 : i + 1);
            printCodecLine(options, index, &item->gpu, "Encoder", item->encoders, item->platformApi);
            printCodecLine(options, index, &item->gpu, "Decoder", item->decoders, item->platformApi);
        }
    } else {
        FFCodecResult merged = {
            .gpu = ffStrbufCreate(),
            .decoders = FF_CODEC_TYPE_NONE,
            .encoders = FF_CODEC_TYPE_NONE,
        };

        FF_LIST_FOR_EACH (FFCodecResult, item, result) {
            merged.decoders |= item->decoders;
            merged.encoders |= item->encoders;
            merged.platformApi = item->platformApi;
        }
        printCodecLine(options, 0, &merged.gpu, "Encoder", merged.encoders, merged.platformApi);
        printCodecLine(options, 0, &merged.gpu, "Decoder", merged.decoders, merged.platformApi);
    }

    FF_LIST_FOR_EACH (FFCodecResult, item, result) {
        ffStrbufDestroy(&item->gpu);
    }
    return true;
}

void ffParseCodecJsonObject(FFCodecOptions* options, yyjson_val* module) {
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach (module, idx, max, key, val) {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs)) {
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "splitGPU")) {
            options->splitGPU = yyjson_get_bool(val);
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "useVulkan")) {
            options->useVulkan = yyjson_get_bool(val);
            continue;
        }

        if (unsafe_yyjson_equals_str(key, "showType")) {
            int value;
            const char* error = ffJsonConfigParseEnum(val, &value, (FFKeyValuePair[]){
                                                                       { "encoder", FF_CODEC_SHOW_TYPE_ENCODER },
                                                                       { "decoder", FF_CODEC_SHOW_TYPE_DECODER },
                                                                       { "both", FF_CODEC_SHOW_TYPE_BOTH },
                                                                       {},
                                                                   });
            if (error) {
                ffPrintError(FF_CODEC_MODULE_NAME, 0, NULL, FF_PRINT_TYPE_NO_CUSTOM_KEY, "Invalid %s value: %s", unsafe_yyjson_get_str(key), error);
            } else {
                options->showType = (FFCodecShowType) value;
            }
            continue;
        }

        ffPrintError(FF_CODEC_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateCodecJsonConfig(FFCodecOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module) {
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);
    yyjson_mut_obj_add_bool(doc, module, "splitGPU", options->splitGPU);
    yyjson_mut_obj_add_bool(doc, module, "useVulkan", options->useVulkan);
    switch (options->showType) {
        case FF_CODEC_SHOW_TYPE_ENCODER:
            yyjson_mut_obj_add_str(doc, module, "showType", "encoder");
            break;
        case FF_CODEC_SHOW_TYPE_DECODER:
            yyjson_mut_obj_add_str(doc, module, "showType", "decoder");
            break;
        case FF_CODEC_SHOW_TYPE_BOTH:
            yyjson_mut_obj_add_str(doc, module, "showType", "both");
            break;
        default:
            break;
    }

}

bool ffGenerateCodecJsonResult(FFCodecOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module) {
    FF_LIST_AUTO_DESTROY result = ffListCreate();
    const char* error = ffDetectCodec(options, &result);

    if (error) {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return false;
    }

    yyjson_mut_val* arr = yyjson_mut_obj_add_arr(doc, module, "result");

    FF_LIST_FOR_EACH (FFCodecResult, item, result) {
        yyjson_mut_val* obj = yyjson_mut_arr_add_obj(doc, arr);
        yyjson_mut_obj_add_strbuf(doc, obj, "gpu", &item->gpu);

        yyjson_mut_val* encoders = yyjson_mut_obj_add_arr(doc, obj, "encoders");
        yyjson_mut_val* decoders = yyjson_mut_obj_add_arr(doc, obj, "decoders");

        for (FFCodecType type = FF_CODEC_TYPE_UNKNOWN; type <= FF_CODEC_TYPE_MAX; type <<= 1) {
            if (item->encoders & type) {
                yyjson_mut_arr_add_str(doc, encoders, ffCodecTypeToString(type));
            }
            if (item->decoders & type) {
                yyjson_mut_arr_add_str(doc, decoders, ffCodecTypeToString(type));
            }
        }
        yyjson_mut_obj_add_str(doc, obj, "platformApi", item->platformApi);
    }

    FF_LIST_FOR_EACH (FFCodecResult, item, result) {
        ffStrbufDestroy(&item->gpu);
    }
    return true;
}

void ffInitCodecOptions(FFCodecOptions* options) {
    ffOptionInitModuleArg(&options->moduleArgs, "󰈫");
    options->splitGPU = false;
    options->useVulkan = false;
    options->showType = FF_CODEC_SHOW_TYPE_BOTH;
}

void ffDestroyCodecOptions(FFCodecOptions* options) {
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffCodecModuleInfo = {
    .name = FF_CODEC_MODULE_NAME,
    .description = "Print hardware video acceleration codec types (decode / encode)",
    .initOptions = (void*) ffInitCodecOptions,
    .destroyOptions = (void*) ffDestroyCodecOptions,
    .parseJsonObject = (void*) ffParseCodecJsonObject,
    .printModule = (void*) ffPrintCodec,
    .generateJsonResult = (void*) ffGenerateCodecJsonResult,
    .generateJsonConfig = (void*) ffGenerateCodecJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]){
        { "GPU name", "gpu" },
        { "Decoder / Encoder", "direction" },
        { "Compatibility alias of codec types", "types" },
        { "Platform API used for detection", "platform-api" },
    }))
};
