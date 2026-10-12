
#include "fastfetch.h"
#include "opengl.h"
#include "common/debug.h"

#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#include <OpenGL/OpenGL.h> // This brings in CGL, not GL

void ffOpenGLHandleResult(FFOpenGLResult* result, typeof(&glGetString) ffglGetString);

static const char* cglHandleContext(FFOpenGLResult* result, CGLContextObj context) {
    CGLError setContextError = CGLSetCurrentContext(context);
    if (setContextError != kCGLNoError) {
        FF_DEBUG("CGLSetCurrentContext() failed: CGLError %d", (int) setContextError);
        return "CGLSetCurrentContext() failed";
    }

    ffOpenGLHandleResult(result, &glGetString);

    GLint major, minor;
    CGLGetVersion(&major, &minor);
    ffStrbufSetF(&result->library, "CGL %d.%d", major, minor);

    return nullptr;
}

static const char* cglHandlePixelFormat(FFOpenGLResult* result, CGLPixelFormatObj pixelFormat) {
    CGLContextObj context;

    CGLError createContextError = CGLCreateContext(pixelFormat, nullptr, &context);
    if (createContextError != kCGLNoError) {
        FF_DEBUG("CGLCreateContext() failed: CGLError %d", (int) createContextError);
        return "CGLCreateContext() failed";
    }

    const char* error = cglHandleContext(result, context);
    CGLDestroyContext(context);
    return error;
}

const char* cglDetectOpenGL(FFOpenGLResult* result) {
    CGLPixelFormatObj pixelFormat;
    CGLPixelFormatAttribute attrs[] = {
        kCGLPFAOpenGLProfile, (CGLPixelFormatAttribute) kCGLOGLPVersion_3_2_Core, kCGLPFAAccelerated, 0
    };

    GLint num;
    CGLError choosePixelFormatError = CGLChoosePixelFormat(attrs, &pixelFormat, &num);
    if (choosePixelFormatError != kCGLNoError) {
        FF_DEBUG("CGLChoosePixelFormat() failed: CGLError %d", (int) choosePixelFormatError);
        return "CGLChoosePixelFormat() failed";
    }

    const char* error = cglHandlePixelFormat(result, pixelFormat);
    CGLDestroyPixelFormat(pixelFormat);
    return error;
}

const char* ffDetectOpenGL(FFOpenGLOptions* options, FFOpenGLResult* result) {
    if (options->library == FF_OPENGL_LIBRARY_AUTO) {
        return cglDetectOpenGL(result);
    } else if (options->library == FF_OPENGL_LIBRARY_EGL) {
#if __has_include(<EGL/egl.h>)
        const char* ffOpenGLDetectByEGL(FFOpenGLResult * result);
        return ffOpenGLDetectByEGL(result);
#else
        return "fastfetch was compiled without egl support";
#endif
    } else {
        FF_DEBUG("Unsupported OpenGL library requested: %d", (int) options->library);
        return "Unsupported OpenGL library";
    }
}
