#include "fastfetch.h"
#include "common/printing.h"
#include "common/parsing.h"

#include <string.h>

#define FF_OPENGL_MODULE_NAME "OpenGL"
#define FF_OPENGL_NUM_FORMAT_ARGS 4

#if defined(FF_HAVE_EGL) || defined(FF_HAVE_GLX) || defined(FF_HAVE_OSMESA)
#define FF_HAVE_GL 1
#include "common/library.h"
#include <GL/gl.h>

#define FF_OPENGL_BUFFER_WIDTH 1
#define FF_OPENGL_BUFFER_HEIGHT 1

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
    const char* slv = (const char*) data->ffglGetString(GL_SHADING_LANGUAGE_VERSION);

    if(instance->config.openGL.outputFormat.length == 0)
    {
        ffPrintLogoAndKey(instance, FF_OPENGL_MODULE_NAME, 0, &instance->config.openGL.key);
        puts(version);
    }
    else
    {
        ffPrintFormat(instance, FF_OPENGL_MODULE_NAME, 0, &instance->config.openGL, FF_OPENGL_NUM_FORMAT_ARGS, (FFformatarg[]) {
            {FF_FORMAT_ARG_TYPE_STRING, version},
            {FF_FORMAT_ARG_TYPE_STRING, renderer},
            {FF_FORMAT_ARG_TYPE_STRING, vendor},
            {FF_FORMAT_ARG_TYPE_STRING, slv}
        });
    }

    return NULL;
}

#ifdef FF_HAVE_EGL
#include <EGL/egl.h>

typedef struct EGLData
{
    GLData glData;

    FF_LIBRARY_SYMBOL(eglGetProcAddress);
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

    return glHandlePrint(instance, &data->glData);
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
        EGL_WIDTH, FF_OPENGL_BUFFER_WIDTH,
        EGL_HEIGHT, FF_OPENGL_BUFFER_HEIGHT,
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
    data->glData.ffglGetString = (__typeof__(data->glData.ffglGetString)) data->ffeglGetProcAddress("glGetString");
    if(!data->glData.ffglGetString)
        return "eglGetProcAddress(glGetString) returned NULL";

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

static const char* eglPrint(FFinstance* instance)
{
    EGLData eglData;

    FF_LIBRARY_LOAD(egl, instance->config.libEGL, "dlopen egl failed", "libEGL.so", 1);
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(egl, eglData.ffeglGetProcAddress, eglGetProcAddress, "dlsym eglGetProcAddress failed");
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
    GLData glData;

    FF_LIBRARY_SYMBOL(glXGetProcAddress);
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

    return glHandlePrint(instance, &data->glData);
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
    data->pixmap = data->ffXCreatePixmap(data->display, DefaultRootWindow(data->display), FF_OPENGL_BUFFER_WIDTH, FF_OPENGL_BUFFER_HEIGHT, (unsigned int) data->visualInfo->depth);
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
    data->glData.ffglGetString = (__typeof__(data->glData.ffglGetString)) data->ffglXGetProcAddress((const GLubyte*) "glGetString");
    if(data->glData.ffglGetString == NULL)
        return "glXGetProcAddress(glGetString) returned NULL";

    data->display = data->ffXOpenDisplay(NULL);
    if(data->display == NULL)
        return "XOpenDisplay returned NULL";

    const char* error = glxHandleDisplay(instance, data);
    data->ffXCloseDisplay(data->display);
    return error;
}

static const char* glxPrint(FFinstance* instance)
{
    GLXData data;

    FF_LIBRARY_LOAD(glx, instance->config.libGLX, "dlopen glx failed", "libGLX.so", 1);
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(glx, data.ffglXGetProcAddress, glXGetProcAddress, "dlsym glXGetProcAddress failed");
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

#ifdef FF_HAVE_OSMESA
#include <GL/osmesa.h>

typedef struct OSMesaData
{
    GLData glData;

    FF_LIBRARY_SYMBOL(OSMesaGetProcAddress);
    FF_LIBRARY_SYMBOL(OSMesaCreateContext);
    FF_LIBRARY_SYMBOL(OSMesaMakeCurrent);
    FF_LIBRARY_SYMBOL(OSMesaDestroyContext);

    OSMesaContext context;
} OSMesaData;

static const char* osMesaHandleContext(FFinstance* instance, OSMesaData* data)
{
    unsigned char buffer[FF_OPENGL_BUFFER_WIDTH * FF_OPENGL_BUFFER_HEIGHT * sizeof(uint32_t)]; // 4 bytes per pixel (RGBA)

    if(data->ffOSMesaMakeCurrent(data->context, buffer, GL_UNSIGNED_BYTE, FF_OPENGL_BUFFER_WIDTH, FF_OPENGL_BUFFER_HEIGHT) != GL_TRUE)
        return "OSMesaMakeCurrent returned GL_FALSE";

    return glHandlePrint(instance, &data->glData);
}

static const char* osMesaHandleData(FFinstance* instance, OSMesaData* data)
{
    //The case to void* is required here, because OSMESAproc can't be cast to (__typeof__(data->glData.ffglGetString)) without a warning, even though it is the actual type.
    data->glData.ffglGetString = (__typeof__(data->glData.ffglGetString)) (void*) data->ffOSMesaGetProcAddress("glGetString");
    if(data->glData.ffglGetString == NULL)
        return "OSMesaGetProcAddress(glGetString) returned NULL";

    data->context = data->ffOSMesaCreateContext(OSMESA_RGBA, NULL);
    if(data->context == NULL)
        return "OSMesaCreateContext returned NULL";

    const char* error = osMesaHandleContext(instance, data);
    data->ffOSMesaDestroyContext(data->context);
    return error;
}

static const char* osMesaPrint(FFinstance* instance)
{
    OSMesaData data;

    FF_LIBRARY_LOAD(osmesa, instance->config.libOSMesa, "dlopen osmesa failed", "libOSMesa.so", 8);
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(osmesa, data.ffOSMesaGetProcAddress, OSMesaGetProcAddress, "dlsym OSMesaGetProcAddress returned NULL");
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(osmesa, data.ffOSMesaCreateContext, OSMesaCreateContext, "dlsym OSMesaCreateContext returned NULL");
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(osmesa, data.ffOSMesaMakeCurrent, OSMesaMakeCurrent, "dlsym OSMesaMakeCurrent returned NULL");
    FF_LIBRARY_LOAD_SYMBOL_ADRESS(osmesa, data.ffOSMesaDestroyContext, OSMesaDestroyContext, "dlsym OSMesaDestroyContext returned NULL");

    const char* error = osMesaHandleData(instance, &data);
    dlclose(osmesa);
    return error;
}

#endif //FF_HAVE_OSMESA

static const char* glPrint(FFinstance* instance)
{
    if(instance->config.glType == FF_GL_TYPE_GLX)
    {
        #ifdef FF_HAVE_GLX
            return glxPrint(instance);
        #else
            return "fastfetch was compiled without glx support";
        #endif
    }

    if(instance->config.glType == FF_GL_TYPE_EGL)
    {
        #ifdef FF_HAVE_EGL
            return eglPrint(instance);
        #else
            return "fastfetch was compiled without egl support";
        #endif
    }

    if(instance->config.glType == FF_GL_TYPE_OSMESA)
    {
        #ifdef FF_HAVE_OSMESA
            return osMesaPrint(instance);
        #else
            return "fastfetch was compiled without osmesa support";
        #endif
    }

    const char* error = ""; // not NULL dummy value

    #ifdef FF_HAVE_EGL
        error = eglPrint(instance);
    #endif

    #ifdef FF_HAVE_GLX
        if(error != NULL)
            error = glxPrint(instance);
    #endif

    //We don't use osmesa in auto mode here, because it is a software implementation,
    //that doesn't reflect the opengl supported by the hardware

    return error;
}

#endif // FF_HAVE_GL

void ffPrintOpenGL(FFinstance* instance)
{
    const char* error;

    #ifndef FF_HAVE_GL
        error = "Fastfetch was built without gl support.";
    #else
       error = glPrint(instance);
    #endif

    if(error != NULL)
        ffPrintError(instance, FF_OPENGL_MODULE_NAME, 0, &instance->config.openGL, error);
}
