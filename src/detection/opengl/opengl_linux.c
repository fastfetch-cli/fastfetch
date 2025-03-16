#include "fastfetch.h"
#include "opengl.h"
#include "common/io/io.h"

#include <string.h>

#if __ANDROID__ && !defined(FF_HAVE_EGL)
    // On Android, installing OpenGL headers is enough (mesa-dev)
    #if __has_include(<EGL/egl.h>)
        #define FF_HAVE_EGL 1
    #endif
#endif

#if defined(FF_HAVE_EGL) || defined(FF_HAVE_GLX)
#define FF_HAVE_GL 1

#include "common/library.h"

#include <GL/gl.h>

void ffOpenGLHandleResult(FFOpenGLResult* result, __typeof__(&glGetString) ffglGetString);

#endif // FF_HAVE_GL

#ifdef FF_HAVE_GLX
#include <GL/glx.h>

typedef struct GLXData
{
    FF_LIBRARY_SYMBOL(glGetString)
    FF_LIBRARY_SYMBOL(glXGetProcAddress)
    FF_LIBRARY_SYMBOL(glXQueryVersion)
    FF_LIBRARY_SYMBOL(XOpenDisplay)
    FF_LIBRARY_SYMBOL(glXChooseVisual)
    FF_LIBRARY_SYMBOL(XCreatePixmap)
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
    ffOpenGLHandleResult(result, data->ffglGetString);

    int major, minor;
    if (data->ffglXQueryVersion(data->display, &major, &minor))
        ffStrbufSetF(&result->library, "GLX %d.%d", major, minor);
    else
        ffStrbufSetStatic(&result->library, "GLX");

    return NULL;
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
    data->ffglGetString = (__typeof__(data->ffglGetString)) data->ffglXGetProcAddress((const GLubyte*) "glGetString");
    if(data->ffglGetString == NULL)
        return "glXGetProcAddress(glGetString) returned NULL";

    data->display = data->ffXOpenDisplay(NULL);
    if(data->display == NULL)
        return "XOpenDisplay returned NULL";

    const char* error = glxHandleDisplay(result, data);
    data->ffXCloseDisplay(data->display);
    return error;
}

static const char* detectByGlx(FFOpenGLResult* result)
{
    GLXData data;

    FF_LIBRARY_LOAD(glx, "dlopen glx failed",
        #if !__OpenBSD__ && !__NetBSD__
            "libGLX"
        #else
            "libGL"
        #endif
            FF_LIBRARY_EXTENSION, 1);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(glx, data, glXGetProcAddress);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(glx, data, glXQueryVersion);
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

    FF_SUPPRESS_IO();

    return glxHandleData(result, &data);
}

#endif //FF_HAVE_GLX

const char* ffDetectOpenGL(FFOpenGLOptions* options, FFOpenGLResult* result)
{
    #if FF_HAVE_GL

    if(options->library == FF_OPENGL_LIBRARY_GLX)
    {
        #ifdef FF_HAVE_GLX
            return detectByGlx(result);
        #else
            return "fastfetch was compiled without glx support";
        #endif
    }

    if(options->library == FF_OPENGL_LIBRARY_EGL)
    {
        #ifdef FF_HAVE_EGL
            const char* ffOpenGLDetectByEGL(FFOpenGLResult* result);
            return ffOpenGLDetectByEGL(result);
        #else
            return "fastfetch was compiled without egl support";
        #endif
    }

    const char* error = ""; // not NULL dummy value

    #ifdef FF_HAVE_EGL
        const char* ffOpenGLDetectByEGL(FFOpenGLResult* result);
        error = ffOpenGLDetectByEGL(result);
    #endif

    #ifdef FF_HAVE_GLX
        if(error != NULL)
            error = detectByGlx(result);
    #endif

    return error;

    #else

        FF_UNUSED(options, result);
        return "Fastfetch was built without gl support.";

    #endif //FF_HAVE_GL
}
