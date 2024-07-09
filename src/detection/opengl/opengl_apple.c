
#include "fastfetch.h"
#include "opengl.h"

#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#include <OpenGL/OpenGL.h> // This brings in CGL, not GL

static void glHandleResult(FFOpenGLResult* result)
{
    ffStrbufAppendS(&result->version, (const char*) glGetString(GL_VERSION));
    ffStrbufAppendS(&result->renderer, (const char*) glGetString(GL_RENDERER));
    ffStrbufAppendS(&result->vendor, (const char*) glGetString(GL_VENDOR));
    ffStrbufAppendS(&result->slv, (const char*) glGetString(GL_SHADING_LANGUAGE_VERSION));

    GLint major, minor;
    CGLGetVersion(&major, &minor);
    ffStrbufAppendF(&result->library, "CGL %d.%d", major, minor);
}

static const char* cglHandleContext(FFOpenGLResult* result, CGLContextObj context)
{
    if(CGLSetCurrentContext(context) != kCGLNoError)
        return "CGLSetCurrentContext() failed";

    glHandleResult(result);

    return NULL;
}

static const char* cglHandlePixelFormat(FFOpenGLResult* result, CGLPixelFormatObj pixelFormat)
{
    CGLContextObj context;

    if(CGLCreateContext(pixelFormat, NULL, &context) != kCGLNoError)
        return "CGLCreateContext() failed";

    const char* error = cglHandleContext(result, context);
    CGLDestroyContext(context);
    return error;
}

const char* ffDetectOpenGL(FF_MAYBE_UNUSED FFOpenGLOptions* options, FFOpenGLResult* result)
{
    CGLPixelFormatObj pixelFormat;
    CGLPixelFormatAttribute attrs[] = {
        kCGLPFAOpenGLProfile, (CGLPixelFormatAttribute) kCGLOGLPVersion_3_2_Core,
        kCGLPFAAccelerated,
        0
    };

    GLint num;
    if (CGLChoosePixelFormat(attrs, &pixelFormat, &num) != kCGLNoError)
        return "CGLChoosePixelFormat() failed";

    const char* error = cglHandlePixelFormat(result, pixelFormat);
    CGLDestroyPixelFormat(pixelFormat);
    return error;
}
