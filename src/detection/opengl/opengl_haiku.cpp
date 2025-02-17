#include <OpenGLKit.h>

extern "C" {
#include "opengl.h"
#include "common/io/io.h"
#if FF_HAVE_EGL
const char* ffOpenGLDetectByEGL(FFOpenGLResult* result);
#endif
void ffOpenGLHandleResult(FFOpenGLResult* result, __typeof__(&glGetString) ffglGetString);
}

static const char* oglDetectOpenGL(FFOpenGLResult* result)
{
    BApplication app("application/x-vnd.fastfetch-cli-fastfetch");
    FF_SUPPRESS_IO();

    BGLView glView(BRect(), "ff_ogl_view", B_FOLLOW_NONE, B_WILL_DRAW, BGL_RGB);
    auto ffglGetString = (decltype(&glGetString)) glView.GetGLProcAddress("glGetString");
    if (!ffglGetString) return "glView.GetGLProcAddress() failed";
    ffOpenGLHandleResult(result, ffglGetString);
    ffStrbufSetStatic(&result->library, "OpenGLKit");
    return NULL;
}

const char* ffDetectOpenGL(FFOpenGLOptions* options, FFOpenGLResult* result)
{
    if (options->library == FF_OPENGL_LIBRARY_AUTO)
        return oglDetectOpenGL(result);
    else if (options->library == FF_OPENGL_LIBRARY_EGL)
    {
        #if FF_HAVE_EGL
        return ffOpenGLDetectByEGL(result);
        #else
        return "fastfetch was compiled without egl support";
        #endif
    }
    else
        return "Unsupported OpenGL library";
}
