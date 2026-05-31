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

static void ffCodecAppendTypeList(FFlist* list, FFCodecType types) {
    for (FFCodecType type = FF_CODEC_TYPE_UNKNOWN; type <= FF_CODEC_TYPE_MAX; type <<= 1) {
        if ((types & type) == 0) {
            continue;
        }
        ffStrbufInitStatic(FF_LIST_ADD(FFstrbuf, *list), ffCodecTypeToString(type));
    }
}

static void ffCodecAppendTypeJoined(FFstrbuf* buffer, FFCodecType types) {
    for (FFCodecType type = FF_CODEC_TYPE_UNKNOWN; type <= FF_CODEC_TYPE_MAX; type <<= 1) {
        if ((types & type) == 0) {
            continue;
        }
        if (buffer->length > 0) {
            ffStrbufAppendS(buffer, ", ");
        }
        ffStrbufAppendS(buffer, ffCodecTypeToString(type));
    }
}

static void ffDestroyCodecResults(FFlist* result) {
    FF_LIST_FOR_EACH (FFCodecResult, item, *result) {
        ffStrbufDestroy(&item->gpu);
    }
}

static void printCodec(const FFCodecOptions* options, const FFCodecResult* result, uint8_t index) {
    FF_STRBUF_AUTO_DESTROY key = ffStrbufCreate();

    if (options->moduleArgs.key.length == 0) {
        ffStrbufSetF(&key, "%s (%s)", FF_CODEC_MODULE_NAME, result->gpu.chars);
    } else {
        FF_PARSE_FORMAT_STRING_CHECKED(&key, &options->moduleArgs.key, ((FFformatarg[]){
                                                                           FF_ARG(result->gpu, "gpu"),
                                                                           FF_ARG(options->moduleArgs.keyIcon, "icon"),
                                                                           FF_ARG(index, "index"),
                                                                       }));
    }

    if (options->moduleArgs.outputFormat.length == 0) {
        FF_STRBUF_AUTO_DESTROY decodersJoined = ffStrbufCreate();
        FF_STRBUF_AUTO_DESTROY encodersJoined = ffStrbufCreate();
        ffCodecAppendTypeJoined(&decodersJoined, result->decoders);
        ffCodecAppendTypeJoined(&encodersJoined, result->encoders);

        ffPrintLogoAndKey(key.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY);
        printf("Decode: %s | Encode: %s", decodersJoined.length ? decodersJoined.chars : "None", encodersJoined.length ? encodersJoined.chars : "None");
        putchar('\n');
    } else {
        FF_LIST_AUTO_DESTROY decoders = ffListCreate(); // Use list instead of pre-joined string for qjs and lua
        FF_LIST_AUTO_DESTROY encoders = ffListCreate();
        ffCodecAppendTypeList(&decoders, result->decoders);
        ffCodecAppendTypeList(&encoders, result->encoders);

        FF_PRINT_FORMAT_CHECKED(key.chars, 0, &options->moduleArgs, FF_PRINT_TYPE_NO_CUSTOM_KEY, ((FFformatarg[]){
                                                                                                     FF_ARG(result->gpu, "gpu"),
                                                                                                     FF_ARG(decoders, "decoders"),
                                                                                                     FF_ARG(encoders, "encoders"),
                                                                                                     FF_ARG(decoders, "types"),
                                                                                                 }));
        // No need to destroy strings in lists, as they are static strings
    }
}

bool ffPrintCodec(FFCodecOptions* options) {
    FF_LIST_AUTO_DESTROY result = ffListCreate();
    const char* error = ffDetectCodec(NULL, &result);

    if (error) {
        ffPrintError(FF_CODEC_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
        return false;
    }

    if (result.length == 0) {
        ffPrintError(FF_CODEC_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "No hardware video acceleration found");
        return false;
    }

    for (uint32_t i = 0; i < result.length; ++i) {
        uint8_t index = (uint8_t) (result.length == 1 ? 0 : i + 1);
        printCodec(options, FF_LIST_GET(FFCodecResult, result, i), index);
    }

    ffDestroyCodecResults(&result);
    return true;
}

void ffParseCodecJsonObject(FFCodecOptions* options, yyjson_val* module) {
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach (module, idx, max, key, val) {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs)) {
            continue;
        }

        ffPrintError(FF_CODEC_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateCodecJsonConfig(FFCodecOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module) {
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);
}

bool ffGenerateCodecJsonResult(FF_A_UNUSED FFCodecOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module) {
    FF_LIST_AUTO_DESTROY result = ffListCreate();
    const char* error = ffDetectCodec(NULL, &result);

    if (error) {
        yyjson_mut_obj_add_str(doc, module, "error", error);
        return false;
    }

    yyjson_mut_val* arr = yyjson_mut_obj_add_arr(doc, module, "result");

    FF_LIST_FOR_EACH (FFCodecResult, item, result) {
        yyjson_mut_val* obj = yyjson_mut_arr_add_obj(doc, arr);
        yyjson_mut_obj_add_strbuf(doc, obj, "gpu", &item->gpu);

        yyjson_mut_val* decoders = yyjson_mut_obj_add_arr(doc, obj, "decoders");
        yyjson_mut_val* encoders = yyjson_mut_obj_add_arr(doc, obj, "encoders");
        yyjson_mut_val* types = yyjson_mut_obj_add_arr(doc, obj, "types"); // compatibility alias for decoders

        for (FFCodecType type = FF_CODEC_TYPE_UNKNOWN; type <= FF_CODEC_TYPE_MAX; type <<= 1) {
            if ((item->decoders & type) != 0) {
                const char* name = ffCodecTypeToString(type);
                yyjson_mut_arr_add_str(doc, decoders, name);
                yyjson_mut_arr_add_str(doc, types, name);
            }
            if ((item->encoders & type) != 0) {
                yyjson_mut_arr_add_str(doc, encoders, ffCodecTypeToString(type));
            }
        }
    }

    ffDestroyCodecResults(&result);
    return true;
}

void ffInitCodecOptions(FFCodecOptions* options) {
    ffOptionInitModuleArg(&options->moduleArgs, "󰚔");
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
        { "Decoder codec type list", "decoders" },
        { "Encoder codec type list", "encoders" },
        { "Compatibility alias of decoders", "types" },
    }))
};
