#include "opengl.h"
#include "common/library.h"
#include "common/printing.h"

#include <Windows.h>
#include <GL/gl.h>

typedef struct WGLData
{
    FF_LIBRARY_SYMBOL(glGetString)
    FF_LIBRARY_SYMBOL(wglMakeCurrent)
    FF_LIBRARY_SYMBOL(wglCreateContext)
    FF_LIBRARY_SYMBOL(wglDeleteContext)
} WGLData;

void ffOpenGLHandleResult(FFOpenGLResult* result, __typeof__(&glGetString) ffglGetString);

static const char* wglHandleContext(WGLData* wglData, FFOpenGLResult* result, HDC hdc, HGLRC context)
{
    if(wglData->ffwglMakeCurrent(hdc, context) == FALSE)
        return "wglMakeCurrent() failed";
    ffOpenGLHandleResult(result, wglData->ffglGetString);
    ffStrbufSetStatic(&result->library, "WGL 1.0");
    if(wglData->ffwglMakeCurrent(NULL, NULL) == FALSE)
        return "wglMakeCurrent(NULL, NULL) failed";
    return NULL;
}

static const char* wglHandlePixelFormat(WGLData* wglData, FFOpenGLResult* result, HWND hWnd)
{
    HDC hdc = GetDC(hWnd);

    if(hdc == NULL)
        return "GetDC() failed";

    PIXELFORMATDESCRIPTOR pfd = {
        .nSize = sizeof(PIXELFORMATDESCRIPTOR),
        .nVersion = 1,
        .dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        .iPixelType = PFD_TYPE_RGBA,
        .cColorBits = 32,
        .cDepthBits = 24,
        .iLayerType = PFD_MAIN_PLANE
    };
    int pixelFormat = ChoosePixelFormat(hdc, &pfd);
    if(pixelFormat == 0)
    {
        ReleaseDC(hWnd, hdc);
        return "ChoosePixelFormat() failed";
    }

    if(SetPixelFormat(hdc, pixelFormat, &pfd) == FALSE)
    {
        ReleaseDC(hWnd, hdc);
        return "SetPixelFormat() failed";
    }

    HGLRC context = wglData->ffwglCreateContext(hdc);
    if(context == NULL)
    {
        ReleaseDC(hWnd, hdc);
        return "wglCreateContext() failed";
    }

    const char* error = wglHandleContext(wglData, result, hdc, context);
    wglData->ffwglDeleteContext(context);

    ReleaseDC(hWnd, hdc);

    return error;
}

static const char* wglDetectOpenGL(FFOpenGLResult* result)
{
    FF_LIBRARY_LOAD(opengl32, "dlopen opengl32" FF_LIBRARY_EXTENSION " failed", "opengl32" FF_LIBRARY_EXTENSION, 1);

    WGLData data = {};

    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(opengl32, data, wglMakeCurrent);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(opengl32, data, wglCreateContext);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(opengl32, data, wglDeleteContext);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(opengl32, data, glGetString);

    HINSTANCE hInstance = GetModuleHandleW(NULL);

    WNDCLASSW wc = {
        .lpfnWndProc = DefWindowProcW,
        .hInstance = hInstance,
        .hbrBackground = (HBRUSH)COLOR_BACKGROUND,
        .lpszClassName = L"ogl_version_check",
        .style = CS_OWNDC,
    };
    if(!RegisterClassW(&wc))
        return "RegisterClassW() failed";

    HWND hWnd = CreateWindowW(wc.lpszClassName, L"ogl_version_check", 0, 0, 0, FF_OPENGL_BUFFER_WIDTH, FF_OPENGL_BUFFER_HEIGHT, NULL, NULL, hInstance, NULL);
    if(!hWnd)
        return "CreateWindowW() failed";

    const char* error = wglHandlePixelFormat(&data, result, hWnd);

    DestroyWindow(hWnd);
    UnregisterClassW(wc.lpszClassName, hInstance);

    return error;
}


const char* ffDetectOpenGL(FFOpenGLOptions* options, FFOpenGLResult* result)
{
    if (options->library == FF_OPENGL_LIBRARY_AUTO)
        return wglDetectOpenGL(result);
    else if (options->library == FF_OPENGL_LIBRARY_EGL)
    {
        #if __has_include(<EGL/egl.h>)
        const char* ffOpenGLDetectByEGL(FFOpenGLResult* result);
        return ffOpenGLDetectByEGL(result);
        #else
        return "fastfetch was compiled without egl support";
        #endif
    }
    else
        return "Unsupported OpenGL library";
}
