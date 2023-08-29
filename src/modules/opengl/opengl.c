#include "common/printing.h"
#include "common/jsonconfig.h"
#include "detection/opengl/opengl.h"
#include "modules/opengl/opengl.h"
#include "util/stringUtils.h"

#define FF_OPENGL_NUM_FORMAT_ARGS 4

void ffPrintOpenGL(FFOpenGLOptions* options)
{
    FFOpenGLResult result;
    ffStrbufInit(&result.version);
    ffStrbufInit(&result.renderer);
    ffStrbufInit(&result.vendor);
    ffStrbufInit(&result.slv);

    const char* error = ffDetectOpenGL(options, &result);
    if(error)
    {
        ffPrintError(FF_OPENGL_MODULE_NAME, 0, &options->moduleArgs, "%s", error);
        return;
    }

    if(options->moduleArgs.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(FF_OPENGL_MODULE_NAME, 0, &options->moduleArgs, FF_PRINT_TYPE_DEFAULT);
        puts(result.version.chars);
    }
    else
    {
        ffPrintFormat(FF_OPENGL_MODULE_NAME, 0, &options->moduleArgs, FF_OPENGL_NUM_FORMAT_ARGS, (FFformatarg[]) {
            {FF_FORMAT_ARG_TYPE_STRBUF, &result.version},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result.renderer},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result.vendor},
            {FF_FORMAT_ARG_TYPE_STRBUF, &result.slv}
        });
    }

    ffStrbufDestroy(&result.version);
    ffStrbufDestroy(&result.renderer);
    ffStrbufDestroy(&result.vendor);
    ffStrbufDestroy(&result.slv);
}

void ffInitOpenGLOptions(FFOpenGLOptions* options)
{
    ffOptionInitModuleBaseInfo(&options->moduleInfo, FF_OPENGL_MODULE_NAME, ffParseOpenGLCommandOptions, ffParseOpenGLJsonObject, ffPrintOpenGL, NULL);
    ffOptionInitModuleArg(&options->moduleArgs);

    #if defined(__linux__) || defined(__FreeBSD__)
    options->library = FF_OPENGL_LIBRARY_AUTO;
    #endif
}

bool ffParseOpenGLCommandOptions(FFOpenGLOptions* options, const char* key, const char* value)
{
    const char* subKey = ffOptionTestPrefix(key, FF_OPENGL_MODULE_NAME);
    if (!subKey) return false;
    if (ffOptionParseModuleArgs(key, subKey, value, &options->moduleArgs))
        return true;

    #if defined(__linux__) || defined(__FreeBSD__)
    if (ffStrEqualsIgnCase(key, "library"))
    {
        options->library = (FFOpenGLLibrary) ffOptionParseEnum(key, value, (FFKeyValuePair[]) {
            { "auto", FF_OPENGL_LIBRARY_AUTO },
            { "egl", FF_OPENGL_LIBRARY_EGL },
            { "glx", FF_OPENGL_LIBRARY_GLX },
            { "osmesa", FF_OPENGL_LIBRARY_OSMESA },
            {}
        });
    }
    #endif

    return false;
}

void ffDestroyOpenGLOptions(FFOpenGLOptions* options)
{
    ffOptionDestroyModuleArg(&options->moduleArgs);
}

void ffParseOpenGLJsonObject(FFOpenGLOptions* options, yyjson_val* module)
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

        #if defined(__linux__) || defined(__FreeBSD__)
        if (ffStrEqualsIgnCase(key, "library"))
        {
            int value;
            const char* error = ffJsonConfigParseEnum(val, &value, (FFKeyValuePair[]) {
                { "auto", FF_OPENGL_LIBRARY_AUTO },
                { "egl", FF_OPENGL_LIBRARY_EGL },
                { "glx", FF_OPENGL_LIBRARY_GLX },
                { "osmesa", FF_OPENGL_LIBRARY_OSMESA },
                {},
            });
            if (error)
                ffPrintError(FF_OPENGL_MODULE_NAME, 0, &options->moduleArgs, "Invalid %s value: %s", key, error);
            else
                options->library = (FFOpenGLLibrary) value;
            continue;
        }
        #endif

        ffPrintError(FF_OPENGL_MODULE_NAME, 0, &options->moduleArgs, "Unknown JSON key %s", key);
    }
}
