#include "opengl.h"
#include "common/debug.h"
#include "common/library.h"

#if __has_include(<GL/gl.h>)
#include <GL/gl.h>
#elif __APPLE__
#define GL_SILENCE_DEPRECATION 1
#include <OpenGL/gl.h>
#else
#define FF_HAVE_NO_GL 1
#endif

#ifndef FF_HAVE_NO_GL

#ifndef GL_SHADING_LANGUAGE_VERSION // For WGL
    #define GL_SHADING_LANGUAGE_VERSION 0x8B8C
#endif

void ffOpenGLHandleResult(FFOpenGLResult* result, __typeof__(&glGetString) ffglGetString)
{
    ffStrbufAppendS(&result->version, (const char*) ffglGetString(GL_VERSION));
    ffStrbufAppendS(&result->renderer, (const char*) ffglGetString(GL_RENDERER));
    ffStrbufAppendS(&result->vendor, (const char*) ffglGetString(GL_VENDOR));
    ffStrbufAppendS(&result->slv, (const char*) ffglGetString(GL_SHADING_LANGUAGE_VERSION));
}

#if defined(FF_HAVE_EGL) || __has_include(<EGL/egl.h>)
#include "common/io.h"

#define EGL_EGL_PROTOTYPES 1
#define EGL_EGLEXT_PROTOTYPES 1
#include <EGL/egl.h>
#include <EGL/eglext.h>

typedef struct EGLData
{
    FF_LIBRARY_SYMBOL(glGetString)
    FF_LIBRARY_SYMBOL(eglGetProcAddress)
    FF_LIBRARY_SYMBOL(eglGetDisplay)
    FF_LIBRARY_SYMBOL(eglQueryString)
    FF_LIBRARY_SYMBOL(eglInitialize)
    FF_LIBRARY_SYMBOL(eglBindAPI)
    FF_LIBRARY_SYMBOL(eglGetConfigs)
    FF_LIBRARY_SYMBOL(eglCreatePbufferSurface)
    FF_LIBRARY_SYMBOL(eglCreateContext)
    FF_LIBRARY_SYMBOL(eglMakeCurrent)
    FF_LIBRARY_SYMBOL(eglDestroyContext)
    FF_LIBRARY_SYMBOL(eglDestroySurface)
    FF_LIBRARY_SYMBOL(eglTerminate)

    EGLDisplay display;
    EGLConfig config;
    EGLSurface surface;
    EGLContext context;
} EGLData;

static const char* eglHandleContext(FFOpenGLResult* result, EGLData* data)
{
    FF_DEBUG("Making EGL context current");
    if(data->ffeglMakeCurrent(data->display, data->surface, data->surface, data->context) != EGL_TRUE)
    {
        FF_DEBUG("eglMakeCurrent() returned EGL_FALSE");
        return "eglMakeCurrent returned EGL_FALSE";
    }

    ffOpenGLHandleResult(result, data->ffglGetString);
    ffStrbufSetF(&result->library, "EGL %s", data->ffeglQueryString(data->display, EGL_VERSION));
    FF_DEBUG("OpenGL via EGL detected: version='%s', renderer='%s', vendor='%s', slv='%s', library='%s'",
        result->version.chars,
        result->renderer.chars,
        result->vendor.chars,
        result->slv.chars,
        result->library.chars);
    return NULL;
}

static const char* eglHandleSurface(FFOpenGLResult* result, EGLData* data, bool gles)
{
    FF_DEBUG("Creating EGL context (preferred API=%s, client version=%d)", gles ? "OpenGL ES" : "OpenGL", gles ? 2 : 1);
    data->context = data->ffeglCreateContext(data->display, data->config, EGL_NO_CONTEXT, (EGLint[]){
        EGL_CONTEXT_CLIENT_VERSION, gles ? 2 : 1, // Try GLES 2.0+ first
        EGL_NONE
    });
    if(data->context == EGL_NO_CONTEXT && gles) // Some ANGLE builds support GLES 1.1 only
    {
        FF_DEBUG("EGL context creation with GLES 2.x failed, retrying with default attributes (GLES 1.1 fallback)");
        data->context = data->ffeglCreateContext(data->display, data->config, EGL_NO_CONTEXT, (EGLint[]){EGL_NONE});
    }
    if(data->context == EGL_NO_CONTEXT)
    {
        FF_DEBUG("eglCreateContext() returned EGL_NO_CONTEXT");
        return "eglCreateContext returned EGL_NO_CONTEXT";
    }

    FF_DEBUG("EGL context created successfully");

    const char* error = eglHandleContext(result, data);
    FF_DEBUG("eglHandleContext() returns: %s", error ?: "success");

    FF_DEBUG("Destroying EGL context");
    data->ffeglDestroyContext(data->display, data->context);
    return error;
}

static const char* eglHandleDisplay(FFOpenGLResult* result, EGLData* data)
{
    // try use OpenGL API. If failed, use the default API (usually OpenGL ES)
    bool gles = !data->ffeglBindAPI(EGL_OPENGL_API);
    FF_DEBUG("eglBindAPI(EGL_OPENGL_API) %s, effective API=%s",
        gles ? "failed" : "succeeded",
        gles ? "default (usually OpenGL ES)" : "OpenGL");

    EGLint eglConfigCount;
    data->ffeglGetConfigs(data->display, &data->config, 1, &eglConfigCount);
    FF_DEBUG("eglGetConfigs() returned %d config(s)", eglConfigCount);

    if(eglConfigCount == 0)
    {
        FF_DEBUG("No EGL config is available");
        return "eglGetConfigs returned 0 configs";
    }

    FF_DEBUG("Creating EGL pbuffer surface (%dx%d)", FF_OPENGL_BUFFER_WIDTH, FF_OPENGL_BUFFER_HEIGHT);
    data->surface = data->ffeglCreatePbufferSurface(data->display, data->config, (EGLint[]){
        EGL_WIDTH, FF_OPENGL_BUFFER_WIDTH,
        EGL_HEIGHT, FF_OPENGL_BUFFER_HEIGHT,
        EGL_NONE
    });

    if(data->surface == EGL_NO_SURFACE)
    {
        FF_DEBUG("eglCreatePbufferSurface() returned EGL_NO_SURFACE");
        return "eglCreatePbufferSurface returned EGL_NO_SURFACE";
    }

    FF_DEBUG("EGL pbuffer surface created successfully");

    const char* error = eglHandleSurface(result, data, gles);
    FF_DEBUG("eglHandleSurface() returns: %s", error ?: "success");

    FF_DEBUG("Destroying EGL surface");
    data->ffeglDestroySurface(data->display, data->surface);
    return error;
}

static const char* eglHandleData(FFOpenGLResult* result, EGLData* data)
{
    FF_DEBUG("Resolving glGetString via eglGetProcAddress()");
    data->ffglGetString = (__typeof__(&glGetString)) data->ffeglGetProcAddress("glGetString");
    if(!data->ffglGetString)
    {
        FF_DEBUG("eglGetProcAddress('glGetString') returned NULL");
        return "eglGetProcAddress(glGetString) returned NULL";
    }

    #if EGL_VERSION_1_5
    PFNEGLGETPLATFORMDISPLAYEXTPROC ffeglGetPlatformDisplay = (PFNEGLGETPLATFORMDISPLAYEXTPROC) data->ffeglGetProcAddress("eglGetPlatformDisplay");
    if (ffeglGetPlatformDisplay)
    {
        FF_DEBUG("Trying eglGetPlatformDisplay(EGL_PLATFORM_SURFACELESS_MESA)");
        data->display = ffeglGetPlatformDisplay(EGL_PLATFORM_SURFACELESS_MESA, NULL, NULL);
        FF_DEBUG("eglGetPlatformDisplay() %s", data->display == EGL_NO_DISPLAY ? "failed" : "succeeded");
    }
    else
        FF_DEBUG("eglGetPlatformDisplay is unavailable, falling back to eglGetDisplay");

    if(!ffeglGetPlatformDisplay || data->display == EGL_NO_DISPLAY)
    #endif

    {
        FF_DEBUG("Trying eglGetDisplay(EGL_DEFAULT_DISPLAY)");
        data->display = data->ffeglGetDisplay(EGL_DEFAULT_DISPLAY);
        if(data->display == EGL_NO_DISPLAY)
        {
            FF_DEBUG("eglGetDisplay() returned EGL_NO_DISPLAY");
            return "eglGetDisplay returned EGL_NO_DISPLAY";
        }

        FF_DEBUG("eglGetDisplay() succeeded");
    }


    EGLint major, minor;
    if(data->ffeglInitialize(data->display, &major, &minor) == EGL_FALSE)
    {
        FF_DEBUG("eglInitialize() returned EGL_FALSE");
        return "eglInitialize returned EGL_FALSE";
    }

    FF_DEBUG("EGL initialized successfully: %d.%d", major, minor);

    const char* error = eglHandleDisplay(result, data);
    FF_DEBUG("eglHandleDisplay() returns: %s", error ?: "success");

    FF_DEBUG("Terminating EGL display connection");
    data->ffeglTerminate(data->display);
    return error;
}


const char* ffOpenGLDetectByEGL(FFOpenGLResult* result)
{
    FF_DEBUG("Starting OpenGL detection via EGL");
    EGLData eglData;

    FF_LIBRARY_LOAD_MESSAGE(egl, "libEGL" FF_LIBRARY_EXTENSION, 1);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(egl, eglData, eglGetProcAddress);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(egl, eglData, eglGetDisplay);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(egl, eglData, eglQueryString);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(egl, eglData, eglInitialize);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(egl, eglData, eglBindAPI);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(egl, eglData, eglGetConfigs);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(egl, eglData, eglCreatePbufferSurface);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(egl, eglData, eglCreateContext);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(egl, eglData, eglMakeCurrent);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(egl, eglData, eglDestroyContext);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(egl, eglData, eglDestroySurface);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(egl, eglData, eglTerminate);

    FF_DEBUG("Loaded EGL library and required symbols");

    FF_SUPPRESS_IO();
    FF_DEBUG("Suppressed stdout/stderr during EGL probing");

    const char* error = eglHandleData(result, &eglData);
    FF_DEBUG("OpenGL detection via EGL returns: %s", error ?: "success");

    return error;
}

#endif //FF_HAVE_EGL

#endif //FF_HAVE_NO_GL
