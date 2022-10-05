#include "fastfetch.h"
#include "common/printing.h"
#include "detection/opengl/opengl.h"

#define FF_OPENGL_MODULE_NAME "OpenGL"
#define FF_OPENGL_NUM_FORMAT_ARGS 4

void ffPrintOpenGL(FFinstance* instance)
{
    FFOpenGLResult result;
    ffStrbufInit(&result.version);
    ffStrbufInit(&result.renderer);
    ffStrbufInit(&result.vendor);
    ffStrbufInit(&result.slv);

    const char* error = ffDetectOpenGL(instance, &result);
    if(error)
    {
        ffPrintError(instance, FF_OPENGL_MODULE_NAME, 0, &instance->config.openGL, "%s", error);
        return;
    }

    if(instance->config.openGL.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_OPENGL_MODULE_NAME, 0, &instance->config.openGL.key);
        puts(result.version.chars);
    }
    else
    {
        ffPrintFormat(instance, FF_OPENGL_MODULE_NAME, 0, &instance->config.openGL, FF_OPENGL_NUM_FORMAT_ARGS, (FFformatarg[]) {
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
