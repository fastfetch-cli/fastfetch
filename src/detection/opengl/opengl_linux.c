#include "fastfetch.h"
#include "opengl.h"

#include <string.h>

#if defined(FF_HAVE_EGL) || defined(FF_HAVE_GLX) || defined(FF_HAVE_OSMESA)
#define FF_HAVE_GL 1

#include "common/library.h"

#include <GL/gl.h>

#define FF_OPENGL_BUFFER_WIDTH 1
#define FF_OPENGL_BUFFER_HEIGHT 1

typedef struct GLData
{
    FF_LIBRARY_SYMBOL(glGetString)
} GLData;

static const char* glHandleResult(FFOpenGLResult* result, const GLData* data, const char* library)
{
    ffStrbufAppendS(&result->version, (const char*) data->ffglGetString(GL_VERSION));
    ffStrbufAppendS(&result->renderer, (const char*) data->ffglGetString(GL_RENDERER));
    ffStrbufAppendS(&result->vendor, (const char*) data->ffglGetString(GL_VENDOR));
    ffStrbufAppendS(&result->slv, (const char*) data->ffglGetString(GL_SHADING_LANGUAGE_VERSION));
    result->library = library;
    return NULL;
}

#endif // FF_HAVE_GL

#ifdef FF_HAVE_EGL
#include "common/io/io.h"

#include <EGL/egl.h>

typedef struct EGLData
{
    GLData glData;

    FF_LIBRARY_SYMBOL(eglGetProcAddress)
    FF_LIBRARY_SYMBOL(eglGetDisplay)
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

    return glHandleResult(result, &data->glData, "EGL");
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

    const char* error = eglHandleSurface(result, data);
    data->ffeglDestroySurface(data->display, data->surface);
    return error;
}

static const char* eglHandleData(FFOpenGLResult* result, EGLData* data)
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

    const char* error = eglHandleDisplay(result, data);
    data->ffeglTerminate(data->display);
    return error;
}

static const char* eglPrint(FFOpenGLResult* result)
{
    EGLData eglData;

    FF_LIBRARY_LOAD(egl, &instance.config.library.libEGL, "dlopen egl failed", "libEGL" FF_LIBRARY_EXTENSION, 1);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(egl, eglData, eglGetProcAddress);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(egl, eglData, eglGetDisplay);
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

#ifdef FF_HAVE_GLX
#include <GL/glx.h>

typedef struct GLXData
{
    GLData glData;

    FF_LIBRARY_SYMBOL(glXGetProcAddress)
    FF_LIBRARY_SYMBOL(XOpenDisplay)
    FF_LIBRARY_SYMBOL(glXChooseVisual)
    FF_LIBRARY_SYMBOL(XCreatePixmap);
    FF_LIBRARY_SYMBOL(glXCreateGLXPixmap)
    FF_LIBRARY_SYMBOL(glXCreateContext)
    FF_LIBRARY_SYMBOL(glXMakeCurrent)
    FF_LIBRARY_SYMBOL(glXDestroyContext)
    FF_LIBRARY_SYMBOL(glXDestroyGLXPixmap)
    FF_LIBRARY_SYMBOL(XFreePixmap)
    FF_LIBRARY_SYMBOL(XCloseDisplay)
    FF_LIBRARY_SYMBOL(XFree)

    Display* display;
    XVisualInfo* visualInfo;
    Pixmap pixmap;
    GLXPixmap glxPixmap;
    GLXContext context;
} GLXData;

static const char* glxHandleContext(FFOpenGLResult* result, GLXData* data)
{
    if(data->ffglXMakeCurrent(data->display, data->glxPixmap, data->context) != True)
        return "glXMakeCurrent returned False";

    return glHandleResult(result, &data->glData, "GLX");
}

static const char* glxHandleGLXPixmap(FFOpenGLResult* result, GLXData* data)
{
    data->context = data->ffglXCreateContext(data->display, data->visualInfo, NULL, True);
    if(data->context == NULL)
        return "glXCreateContext returned NULL";

    const char* error = glxHandleContext(result, data);
    data->ffglXDestroyContext(data->display, data->context);
    return error;
}

static const char* glxHandlePixmap(FFOpenGLResult* result, GLXData* data)
{
    data->glxPixmap = data->ffglXCreateGLXPixmap(data->display, data->visualInfo, data->pixmap);
    if(data->glxPixmap == None)
        return "glXCreateGLXPixmap returned None";

    const char* error = glxHandleGLXPixmap(result, data);
    data->ffglXDestroyGLXPixmap(data->display, data->glxPixmap);
    return error;
}

static const char* glxHandleVisualInfo(FFOpenGLResult* result, GLXData* data)
{
    data->pixmap = data->ffXCreatePixmap(data->display, DefaultRootWindow(data->display), FF_OPENGL_BUFFER_WIDTH, FF_OPENGL_BUFFER_HEIGHT, (unsigned int) data->visualInfo->depth);
    if(data->pixmap == None)
        return "XCreatePixmap returned None";

    const char* error = glxHandlePixmap(result, data);
    data->ffXFreePixmap(data->display, data->pixmap);
    return error;
}

static const char* glxHandleDisplay(FFOpenGLResult* result, GLXData* data)
{
    data->visualInfo = data->ffglXChooseVisual(data->display, DefaultScreen(data->display), (int[]){None});
    if(data->visualInfo == NULL)
        return "glXChooseVisual returned NULL";

    const char* error = glxHandleVisualInfo(result, data);
    data->ffXFree(data->visualInfo);
    return error;
}

static const char* glxHandleData(FFOpenGLResult* result, GLXData* data)
{
    data->glData.ffglGetString = (__typeof__(data->glData.ffglGetString)) data->ffglXGetProcAddress((const GLubyte*) "glGetString");
    if(data->glData.ffglGetString == NULL)
        return "glXGetProcAddress(glGetString) returned NULL";

    data->display = data->ffXOpenDisplay(NULL);
    if(data->display == NULL)
        return "XOpenDisplay returned NULL";

    const char* error = glxHandleDisplay(result, data);
    data->ffXCloseDisplay(data->display);
    return error;
}

static const char* glxPrint(FFOpenGLResult* result)
{
    GLXData data;

    FF_LIBRARY_LOAD(glx, &instance.config.library.libGLX, "dlopen glx failed", "libGLX" FF_LIBRARY_EXTENSION, 1);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(glx, data, glXGetProcAddress);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(glx, data, XOpenDisplay);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(glx, data, glXChooseVisual);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(glx, data, XCreatePixmap);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(glx, data, glXCreateGLXPixmap);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(glx, data, glXCreateContext);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(glx, data, glXMakeCurrent);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(glx, data, glXDestroyContext);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(glx, data, glXDestroyGLXPixmap);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(glx, data, XFreePixmap);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(glx, data, XCloseDisplay);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(glx, data, XFree);

    return glxHandleData(result, &data);
}

#endif //FF_HAVE_GLX

#ifdef FF_HAVE_OSMESA
#if __has_include(<GL/osmesa.h>)
    #include <GL/osmesa.h>
#else
    #include <mesa/osmesa.h> // for sunos
#endif

typedef struct OSMesaData
{
    GLData glData;

    FF_LIBRARY_SYMBOL(OSMesaGetProcAddress)
    FF_LIBRARY_SYMBOL(OSMesaCreateContext)
    FF_LIBRARY_SYMBOL(OSMesaMakeCurrent)
    FF_LIBRARY_SYMBOL(OSMesaDestroyContext)

    OSMesaContext context;
} OSMesaData;

static const char* osMesaHandleContext(FFOpenGLResult* result, OSMesaData* data)
{
    unsigned char buffer[FF_OPENGL_BUFFER_WIDTH * FF_OPENGL_BUFFER_HEIGHT * sizeof(uint32_t)]; // 4 bytes per pixel (RGBA)

    if(data->ffOSMesaMakeCurrent(data->context, buffer, GL_UNSIGNED_BYTE, FF_OPENGL_BUFFER_WIDTH, FF_OPENGL_BUFFER_HEIGHT) != GL_TRUE)
        return "OSMesaMakeCurrent returned GL_FALSE";

    return glHandleResult(result, &data->glData, "OSMesa");
}

static const char* osMesaHandleData(FFOpenGLResult* result, OSMesaData* data)
{
    //The case to void* is required here, because OSMESAproc can't be cast to (__typeof__(data->glData.ffglGetString)) without a warning, even though it is the actual type.
    data->glData.ffglGetString = (__typeof__(data->glData.ffglGetString)) (void*) data->ffOSMesaGetProcAddress("glGetString");
    if(data->glData.ffglGetString == NULL)
        return "OSMesaGetProcAddress(glGetString) returned NULL";

    data->context = data->ffOSMesaCreateContext(OSMESA_RGBA, NULL);
    if(data->context == NULL)
        return "OSMesaCreateContext returned NULL";

    const char* error = osMesaHandleContext(result, data);
    data->ffOSMesaDestroyContext(data->context);
    return error;
}

static const char* osMesaPrint(FFOpenGLResult* result)
{
    OSMesaData data;

    FF_LIBRARY_LOAD(osmesa, &instance.config.library.libOSMesa, "dlopen osmesa failed", "libOSMesa" FF_LIBRARY_EXTENSION, 8);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(osmesa, data, OSMesaGetProcAddress);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(osmesa, data, OSMesaCreateContext);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(osmesa, data, OSMesaMakeCurrent);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(osmesa, data, OSMesaDestroyContext);

    return osMesaHandleData(result, &data);
}

#endif //FF_HAVE_OSMESA

const char* ffDetectOpenGL(FFOpenGLOptions* options, FFOpenGLResult* result)
{
    #if FF_HAVE_GL

    if(options->library == FF_OPENGL_LIBRARY_GLX)
    {
        #ifdef FF_HAVE_GLX
            return glxPrint(result);
        #else
            return "fastfetch was compiled without glx support";
        #endif
    }

    if(options->library == FF_OPENGL_LIBRARY_EGL)
    {
        #ifdef FF_HAVE_EGL
            return eglPrint(result);
        #else
            return "fastfetch was compiled without egl support";
        #endif
    }

    if(options->library == FF_OPENGL_LIBRARY_OSMESA)
    {
        #ifdef FF_HAVE_OSMESA
            return osMesaPrint(result);
        #else
            return "fastfetch was compiled without osmesa support";
        #endif
    }

    const char* error = ""; // not NULL dummy value

    #ifdef FF_HAVE_EGL
        error = eglPrint(result);
    #endif

    #ifdef FF_HAVE_GLX
        if(error != NULL)
            error = glxPrint(result);
    #endif

    //We don't use osmesa in auto mode here, because it is a software implementation,
    //that doesn't reflect the opengl supported by the hardware

    return error;

    #else

        FF_UNUSED(options, result);
        return "Fastfetch was built without gl support.";

    #endif //FF_HAVE_GL
}
