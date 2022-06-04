#include "fastfetch.h"

#include <string.h>

#define FF_OPENGL_MODULE_NAME "OpenGL"
#define FF_OPENGL_NUM_FORMAT_ARGS 3

#ifdef FF_HAVE_GL
#include <GL/gl.h>

typedef struct GLData
{
    FF_LIBRARY_SYMBOL(glGetString);
} GLData;

static const char* glHandlePrint(FFinstance* instance, const GLData* data)
{
    const char* version = (const char*) data->ffglGetString(GL_VERSION);
    if(!ffStrSet(version))
        return "glGetString(GL_VERSION) returned NULL";

    const char* renderer = (const char*) data->ffglGetString(GL_RENDERER);
    const char* vendor = (const char*) data->ffglGetString(GL_VENDOR);

    if(instance->config.openGLFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_OPENGL_MODULE_NAME, 0, &instance->config.openGLKey);
        puts(version);
    }
    else
    {
        ffPrintFormatString(instance, FF_OPENGL_MODULE_NAME, 0, &instance->config.openGLKey, &instance->config.openGLFormat, NULL, FF_OPENGL_NUM_FORMAT_ARGS, (FFformatarg[]) {
            {FF_FORMAT_ARG_TYPE_STRING, version},
            {FF_FORMAT_ARG_TYPE_STRING, renderer},
            {FF_FORMAT_ARG_TYPE_STRING, vendor}
        });
    }

    return NULL;
}

#ifdef FF_HAVE_EGL
#include <EGL/egl.h>

typedef struct EGLData
{
    GLData* glData;

    FF_LIBRARY_SYMBOL(eglGetDisplay);
    FF_LIBRARY_SYMBOL(eglInitialize);
    FF_LIBRARY_SYMBOL(eglBindAPI);
    FF_LIBRARY_SYMBOL(eglGetConfigs);
    FF_LIBRARY_SYMBOL(eglCreatePbufferSurface);
    FF_LIBRARY_SYMBOL(eglCreateContext);
    FF_LIBRARY_SYMBOL(eglMakeCurrent);
    FF_LIBRARY_SYMBOL(eglDestroyContext);
    FF_LIBRARY_SYMBOL(eglDestroySurface);
    FF_LIBRARY_SYMBOL(eglTerminate);

    EGLDisplay display;
    EGLConfig config;
    EGLSurface surface;
    EGLContext context;
} EGLData;

static const char* eglHandleContext(FFinstance* instance, EGLData* data)
{
    if(data->ffeglMakeCurrent(data->display, data->surface, data->surface, data->context) != EGL_TRUE)
        return "eglMakeCurrent returned EGL_FALSE";

    return glHandlePrint(instance, data->glData);
}

static const char* eglHandleSurface(FFinstance* instance, EGLData* data)
{
    data->context = data->ffeglCreateContext(data->display, data->config, EGL_NO_CONTEXT, (EGLint[]){EGL_NONE});
    if(data->context == EGL_NO_CONTEXT)
        return "eglCreateContext returned EGL_NO_CONTEXT";

    const char* error = eglHandleContext(instance, data);
    data->ffeglDestroyContext(data->display, data->context);
    return error;
}

static const char* eglHandleDisplay(FFinstance* instance, EGLData* data)
{
    if(data->ffeglBindAPI(EGL_OPENGL_API) != EGL_TRUE)
        return "eglBindAPI returned EGL_FALSE";

    EGLint eglConfigCount;
    data->ffeglGetConfigs(data->display, &data->config, 1, &eglConfigCount);
    if(eglConfigCount == 0)
        return "eglGetConfigs returned 0 configs";

    data->surface = data->ffeglCreatePbufferSurface(data->display, data->config, (EGLint[]){
        EGL_WIDTH, 1,
        EGL_HEIGHT, 1,
        EGL_NONE
    });

    if(data->surface == EGL_NO_SURFACE)
        return "eglCreatePbufferSurface returned EGL_NO_SURFACE";

    const char* error = eglHandleSurface(instance, data);
    data->ffeglDestroySurface(data->display, data->surface);
    return error;
}

static const char* eglHandleData(FFinstance* instance, EGLData* data)
{
    data->display = data->ffeglGetDisplay(EGL_DEFAULT_DISPLAY);
    if(data->display == EGL_NO_DISPLAY)
        return "eglGetDisplay returned EGL_NO_DISPLAY";

    EGLint major, minor;
    if(data->ffeglInitialize(data->display, &major, &minor) == EGL_FALSE)
        return "eglInitialize returned EGL_FALSE";

    const char* error = eglHandleDisplay(instance, data);
    data->ffeglTerminate(data->display);
    return error;
}

static const char* eglPrint(FFinstance* instance, GLData* glData)
{
    EGLData eglData;
    eglData.glData = glData;

    FF_LIBRARY_LOAD(egl, instance->config.libEGL, "dlopen egl failed", "libEGL.so", 1);
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(egl, eglData.ffeglGetDisplay, eglGetDisplay, "dlsym eglGetDisplay failed");
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(egl, eglData.ffeglInitialize, eglInitialize, "dlsym eglInitialize failed");
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(egl, eglData.ffeglBindAPI, eglBindAPI, "dlsym eglBindAPI failed");
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(egl, eglData.ffeglGetConfigs, eglGetConfigs, "dlsym eglGetConfigs failed");
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(egl, eglData.ffeglCreatePbufferSurface, eglCreatePbufferSurface, "dlsym eglCreatePbufferSurface failed");
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(egl, eglData.ffeglCreateContext, eglCreateContext, "dlsym eglCreateContext failed");
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(egl, eglData.ffeglMakeCurrent, eglMakeCurrent, "dlsym eglMakeCurrent failed");
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(egl, eglData.ffeglDestroyContext, eglDestroyContext, "dlsym eglDestroyContext failed");
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(egl, eglData.ffeglDestroySurface, eglDestroySurface, "dlsym eglDestroySurface failed");
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(egl, eglData.ffeglTerminate, eglTerminate, "dlsym eglTerminate failed");

    const char* error = eglHandleData(instance, &eglData);
    dlclose(egl);
    return error;
}

#endif //FF_HAVE_EGL

#ifdef FF_HAVE_GLX
#include <GL/glx.h>

typedef struct GLXData
{
    GLData* glData;

    FF_LIBRARY_SYMBOL(XOpenDisplay);
    FF_LIBRARY_SYMBOL(glXChooseVisual);
    FF_LIBRARY_SYMBOL(XCreatePixmap);
    FF_LIBRARY_SYMBOL(glXCreateGLXPixmap);
    FF_LIBRARY_SYMBOL(glXCreateContext);
    FF_LIBRARY_SYMBOL(glXMakeCurrent);
    FF_LIBRARY_SYMBOL(glXDestroyContext);
    FF_LIBRARY_SYMBOL(glXDestroyGLXPixmap);
    FF_LIBRARY_SYMBOL(XFreePixmap);
    FF_LIBRARY_SYMBOL(XCloseDisplay);

    Display* display;
    XVisualInfo* visualInfo;
    Pixmap pixmap;
    GLXPixmap glxPixmap;
    GLXContext context;
} GLXData;

static const char* glxHandleContext(FFinstance* instance, GLXData* data)
{
    if(data->ffglXMakeCurrent(data->display, data->glxPixmap, data->context) != True)
        return "glXMakeCurrent returned False";

    return glHandlePrint(instance, data->glData);
}

static const char* glxHandleGLXPixmap(FFinstance* instance, GLXData* data)
{
    data->context = data->ffglXCreateContext(data->display, data->visualInfo, NULL, True);
    if(data->context == NULL)
        return "glXCreateContext returned NULL";

    const char* error = glxHandleContext(instance, data);
    data->ffglXDestroyContext(data->display, data->context);
    return error;
}

static const char* glxHandlePixmap(FFinstance* instance, GLXData* data)
{
    data->glxPixmap = data->ffglXCreateGLXPixmap(data->display, data->visualInfo, data->pixmap);
    if(data->glxPixmap == None)
        return "glXCreateGLXPixmap returned None";

    const char* error = glxHandleGLXPixmap(instance, data);
    data->ffglXDestroyGLXPixmap(data->display, data->glxPixmap);
    return error;
}

static const char* glxHandleVisualInfo(FFinstance* instance, GLXData* data)
{
    data->pixmap = data->ffXCreatePixmap(data->display, DefaultRootWindow(data->display), 1, 1, (unsigned int) data->visualInfo->depth);
    if(data->pixmap == None)
        return "XCreatePixmap returned None";

    const char* error = glxHandlePixmap(instance, data);
    data->ffXFreePixmap(data->display, data->pixmap);
    return error;
}

static const char* glxHandleDisplay(FFinstance* instance, GLXData* data)
{
    data->visualInfo = data->ffglXChooseVisual(data->display, DefaultScreen(data->display), (int[]){None});
    if(data->visualInfo == NULL)
        return "glXChooseVisual returned NULL";

    return glxHandleVisualInfo(instance, data);
}

static const char* glxHandleData(FFinstance* instance, GLXData* data)
{
    data->display = data->ffXOpenDisplay(NULL);
    if(data->display == NULL)
        return "XOpenDisplay returned NULL";

    const char* error = glxHandleDisplay(instance, data);
    data->ffXCloseDisplay(data->display);
    return error;
}

static const char* glxPrint(FFinstance* instance, GLData* glData)
{
    GLXData data;
    data.glData = glData;

    FF_LIBRARY_LOAD(glx, instance->config.libGLX, "dlopen glx failed", "libGLX.so", 1);
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(glx, data.ffXOpenDisplay, XOpenDisplay, "dlsym XOpenDisplay returned NULL");
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(glx, data.ffglXChooseVisual, glXChooseVisual, "dlsym glXChooseVisual returned NULL");
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(glx, data.ffXCreatePixmap, XCreatePixmap, "dlsym XCreatePixmap returned NULL");
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(glx, data.ffglXCreateGLXPixmap, glXCreateGLXPixmap, "dlsym glXCreateGLXPixmap returned NULL");
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(glx, data.ffglXCreateContext, glXCreateContext, "dlsym glXCreateContext returned NULL");
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(glx, data.ffglXMakeCurrent, glXMakeCurrent, "dlsym glXMakeCurrent returned NULL");
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(glx, data.ffglXDestroyContext, glXDestroyContext, "dlsym glXDestroyContext returned NULL");
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(glx, data.ffglXDestroyGLXPixmap, glXDestroyGLXPixmap, "dlsym glXDestroyGLXPixmap returned NULL");
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(glx, data.ffXFreePixmap, XFreePixmap, "dlsym XFreePixmap returned NULL");
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(glx, data.ffXCloseDisplay, XCloseDisplay, "dlsym XCloseDisplay returned NULL");

    const char* error = glxHandleData(instance, &data);
    dlclose(glx);
    return error;
}

#endif //FF_HAVE_GLX

static const char* glHandleGL(FFinstance* instance, GLData* data)
{
    if(instance->config.glType == FF_GL_TYPE_GLX)
    {
        #ifdef FF_HAVE_GL
            return glxPrint(instance, data);
        #else
            return = "fastfetch was compiled without glx support";
        #endif
    }

    if(instance->config.glType == FF_GL_TYPE_EGL)
    {
        #ifdef FF_HAVE_EGL
            return eglPrint(instance, data);
        #else
            return "fastfetch was compiled without egl support";
        #endif
    }

    const char* error = ""; // not NULL dummy value

    #ifdef FF_HAVE_EGL
        error = eglPrint(instance, data);
    #endif

    #ifdef FF_HAVE_GLX
        if(error != NULL)
            error = glxPrint(instance, data);
    #endif

    return error;
}

static const char* glPrint(FFinstance* instance)
{
    GLData data;

    FF_LIBRARY_LOAD(gl, instance->config.libGL, "dlopen gl failed", "libGL.so", 1);
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(gl, data.ffglGetString, glGetString, "dlsym glGetString failed");

    const char* error = glHandleGL(instance, &data);

    dlclose(gl);
    return error;
}

#endif //FF_HAVE_GL

void ffPrintOpenGL(FFinstance* instance)
{
    const char* error;

    #ifndef FF_HAVE_GL
        error = "Fastfetch was built without gl support.";
    #else
       error = glPrint(instance);
    #endif

    if(error != NULL)
        ffPrintError(instance, FF_OPENGL_MODULE_NAME, 0, &instance->config.openGLKey, &instance->config.openGLFormat, FF_OPENGL_NUM_FORMAT_ARGS, error);
}
