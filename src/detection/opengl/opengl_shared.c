#include "opengl.h"
#include "common/library.h"

#if __has_include(<GL/gl.h>)
#include <GL/gl.h>
#elif __has_include(<OpenGL/gl.h>)
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
#include "common/io/io.h"

#include <EGL/egl.h>

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
    if(data->ffeglMakeCurrent(data->display, data->surface, data->surface, data->context) != EGL_TRUE)
        return "eglMakeCurrent returned EGL_FALSE";

    ffOpenGLHandleResult(result, data->ffglGetString);
    ffStrbufSetF(&result->library, "EGL %s", data->ffeglQueryString(data->display, EGL_VERSION));
    return NULL;
}

static const char* eglHandleSurface(FFOpenGLResult* result, EGLData* data)
{
    data->context = data->ffeglCreateContext(data->display, data->config, EGL_NO_CONTEXT, (EGLint[]){EGL_NONE});
    if(data->context == EGL_NO_CONTEXT)
        return "eglCreateContext returned EGL_NO_CONTEXT";

    const char* error = eglHandleContext(result, data);
    data->ffeglDestroyContext(data->display, data->context);
    return error;
}

static const char* eglHandleDisplay(FFOpenGLResult* result, EGLData* data)
{
    // try use OpenGL API. If failed, use the default API (usually OpenGL ES)
    data->ffeglBindAPI(EGL_OPENGL_API);

    EGLint eglConfigCount;
    data->ffeglGetConfigs(data->display, &data->config, 1, &eglConfigCount);
    if(eglConfigCount == 0)
        return "eglGetConfigs returned 0 configs";

    data->surface = data->ffeglCreatePbufferSurface(data->display, data->config, (EGLint[]){
        EGL_WIDTH, FF_OPENGL_BUFFER_WIDTH,
        EGL_HEIGHT, FF_OPENGL_BUFFER_HEIGHT,
        EGL_NONE
    });

    if(data->surface == EGL_NO_SURFACE)
        return "eglCreatePbufferSurface returned EGL_NO_SURFACE";

    const char* error = eglHandleSurface(result, data);
    data->ffeglDestroySurface(data->display, data->surface);
    return error;
}

static const char* eglHandleData(FFOpenGLResult* result, EGLData* data)
{
    data->ffglGetString = (__typeof__(&glGetString)) data->ffeglGetProcAddress("glGetString");
    if(!data->ffglGetString)
        return "eglGetProcAddress(glGetString) returned NULL";

    data->display = data->ffeglGetDisplay(EGL_DEFAULT_DISPLAY);
    if(data->display == EGL_NO_DISPLAY)
        return "eglGetDisplay returned EGL_NO_DISPLAY";

    EGLint major, minor;
    if(data->ffeglInitialize(data->display, &major, &minor) == EGL_FALSE)
        return "eglInitialize returned EGL_FALSE";

    const char* error = eglHandleDisplay(result, data);
    data->ffeglTerminate(data->display);
    return error;
}


const char* ffOpenGLDetectByEGL(FFOpenGLResult* result)
{
    EGLData eglData;

    FF_LIBRARY_LOAD(egl, "dlopen libEGL" FF_LIBRARY_EXTENSION " failed", "libEGL" FF_LIBRARY_EXTENSION, 1);
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

    FF_SUPPRESS_IO();
    return eglHandleData(result, &eglData);
}

#endif //FF_HAVE_EGL

#endif //FF_HAVE_NO_GL
