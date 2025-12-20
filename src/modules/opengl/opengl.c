#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/opengl/opengl.h"
#include "modules/opengl/opengl.h"
#include "util/stringUtils.h"

bool ffPrintOpenGL(FFOpenGLOptions* options)
{
    bool success = false;
    FFOpenGLResult result;
    ffStrbufInit(&result.version);
    ffStrbufInit(&result.renderer);
    ffStrbufInit(&result.vendor);
    ffStrbufInit(&result.slv);
    ffStrbufInit(&result.library);

    const char* error = ffDetectOpenGL(options, &result);
    if(error)
    {
        ffPrintError(FF_OPENGL_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "%s", error);
    }
    else
    {
        if(options->moduleArgs.outputFormat.length == 0)
        {
            ffPrintLogoAndKey(FF_OPENGL_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
            puts(result.version.chars);
        }
        else
        {
            FF_PRINT_FORMAT_CHECKED(FF_OPENGL_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, ((FFformatarg[]) {
                FF_FORMAT_ARG(result.version, "version"),
                FF_FORMAT_ARG(result.renderer, "renderer"),
                FF_FORMAT_ARG(result.vendor, "vendor"),
                FF_FORMAT_ARG(result.slv, "slv"),
                FF_FORMAT_ARG(result.library, "library"),
            }));
        }
        success = true;
    }

    ffStrbufDestroy(&result.version);
    ffStrbufDestroy(&result.renderer);
    ffStrbufDestroy(&result.vendor);
    ffStrbufDestroy(&result.slv);
    ffStrbufDestroy(&result.library);

    return success;
}

void ffParseOpenGLJsonObject(FFOpenGLOptions* options, yyjson_val* module)
{
    yyjson_val *key, *val;
    size_t idx, max;
    yyjson_obj_foreach(module, idx, max, key, val)
    {
        if (ffJsonConfigParseModuleArgs(key, val, &options->moduleArgs))
            continue;

        if (unsafe_yyjson_equals_str(key, "library"))
        {
            int value;
            const char* error = ffJsonConfigParseEnum(val, &value, (FFKeyValuePair[]) {
                { "auto", FF_OPENGL_LIBRARY_AUTO },
                { "egl", FF_OPENGL_LIBRARY_EGL },
                { "glx", FF_OPENGL_LIBRARY_GLX },
                {},
            });
            if (error)
                ffPrintError(FF_OPENGL_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Invalid %s value: %s", unsafe_yyjson_get_str(key), error);
            else
                options->library = (FFOpenGLLibrary) value;
            continue;
        }

        ffPrintError(FF_OPENGL_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT, "Unknown JSON key %s", unsafe_yyjson_get_str(key));
    }
}

void ffGenerateOpenGLJsonConfig(FFOpenGLOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    ffJsonConfigGenerateModuleArgsConfig(doc, module, &options->moduleArgs);

    switch (options->library)
    {
    case FF_OPENGL_LIBRARY_AUTO:
        yyjson_mut_obj_add_str(doc, module, "library", "auto");
        break;
    case FF_OPENGL_LIBRARY_EGL:
        yyjson_mut_obj_add_str(doc, module, "library", "egl");
        break;
    case FF_OPENGL_LIBRARY_GLX:
        yyjson_mut_obj_add_str(doc, module, "library", "glx");
        break;
    }
}

bool ffGenerateOpenGLJsonResult(FF_MAYBE_UNUSED FFOpenGLOptions* options, yyjson_mut_doc* doc, yyjson_mut_val* module)
{
    bool success = false;
    FFOpenGLResult result;
    ffStrbufInit(&result.version);
    ffStrbufInit(&result.renderer);
    ffStrbufInit(&result.vendor);
    ffStrbufInit(&result.slv);
    ffStrbufInit(&result.library);

    const char* error = ffDetectOpenGL(options, &result);
    if(error != NULL)
    {
        yyjson_mut_obj_add_str(doc, module, "error", error);
    }
    else
    {
        yyjson_mut_val* obj = yyjson_mut_obj_add_obj(doc, module, "result");
        yyjson_mut_obj_add_strbuf(doc, obj, "version", &result.version);
        yyjson_mut_obj_add_strbuf(doc, obj, "renderer", &result.renderer);
        yyjson_mut_obj_add_strbuf(doc, obj, "vendor", &result.vendor);
        yyjson_mut_obj_add_strbuf(doc, obj, "slv", &result.slv);
        yyjson_mut_obj_add_strbuf(doc, obj, "library", &result.library);
        success = true;
    }

    ffStrbufDestroy(&result.version);
    ffStrbufDestroy(&result.renderer);
    ffStrbufDestroy(&result.vendor);
    ffStrbufDestroy(&result.slv);
    ffStrbufDestroy(&result.library);

    return success;
}

void ffInitOpenGLOptions(FFOpenGLOptions* options)
{
    ffOptionInitModuleArg(&options->moduleArgs, "");

    options->library = FF_OPENGL_LIBRARY_AUTO;
}

void ffDestroyOpenGLOptions(FFOpenGLOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

FFModuleBaseInfo ffOpenGLModuleInfo = {
    .name = FF_OPENGL_MODULE_NAME,
    .description = "Print highest OpenGL version supported by the GPU",
    .initOptions = (void*) ffInitOpenGLOptions,
    .destroyOptions = (void*) ffDestroyOpenGLOptions,
    .parseJsonObject = (void*) ffParseOpenGLJsonObject,
    .printModule = (void*) ffPrintOpenGL,
    .generateJsonResult = (void*) ffGenerateOpenGLJsonResult,
    .generateJsonConfig = (void*) ffGenerateOpenGLJsonConfig,
    .formatArgs = FF_FORMAT_ARG_LIST(((FFModuleFormatArg[]) {
        {"OpenGL version", "version"},
        {"OpenGL renderer", "renderer"},
        {"OpenGL vendor", "vendor"},
        {"OpenGL shading language version", "slv"},
        {"OpenGL library used", "library"},
    }))
};
