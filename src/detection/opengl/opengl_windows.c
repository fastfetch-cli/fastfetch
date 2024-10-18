#include "opengl.h"
#include "common/library.h"
#include "common/printing.h"

#include <Windows.h>
#include <GL/gl.h>

typedef struct WGLData
{
    FFOpenGLResult* result;
    const char* error;

    FF_LIBRARY_SYMBOL(glGetString)
    FF_LIBRARY_SYMBOL(wglMakeCurrent)
    FF_LIBRARY_SYMBOL(wglCreateContext)
    FF_LIBRARY_SYMBOL(wglDeleteContext)
} WGLData;

void ffOpenGLHandleResult(FFOpenGLResult* result, __typeof__(&glGetString) ffglGetString);

static const char* wglHandleContext(WGLData* wglData, HDC hdc, HGLRC context)
{
    if(wglData->ffwglMakeCurrent(hdc, context) == FALSE)
        return "wglMakeCurrent() failed";
    ffOpenGLHandleResult(wglData->result, wglData->ffglGetString);
    ffStrbufSetStatic(&wglData->result->library, "WGL 1.0");
    return NULL;
}

static const char* wglHandlePixelFormat(WGLData* wglData, HWND hWnd)
{
    PIXELFORMATDESCRIPTOR pfd =
    {
        sizeof(pfd),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    //Flags
        PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or palette.
        32,                   // Colordepth of the framebuffer.
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        24,                   // Number of bits for the depthbuffer
        8,                    // Number of bits for the stencilbuffer
        0,                    // Number of Aux buffers in the framebuffer.
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };

    HDC hdc = GetDC(hWnd);

    if(SetPixelFormat(hdc, ChoosePixelFormat(hdc, &pfd), &pfd) == FALSE)
        return "SetPixelFormat() failed";

    HGLRC context = wglData->ffwglCreateContext(hdc);
    if(context == NULL)
        return "wglCreateContext() failed";

    const char* error = wglHandleContext(wglData, hdc, context);
    wglData->ffwglDeleteContext(context);

    return error;
}

static LRESULT CALLBACK wglHandleWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
    case WM_CREATE: {
        WGLData* wglData = (WGLData*)((CREATESTRUCT*)lParam)->lpCreateParams;
        wglData->error = wglHandlePixelFormat(wglData, hWnd);
        PostQuitMessage(0);
        return 0;
    }
    default:
        return DefWindowProcW(hWnd, message, wParam, lParam);
    }
}

static const char* wglDetectOpenGL(FFOpenGLResult* result)
{
    FF_LIBRARY_LOAD(opengl32, "dlopen opengl32" FF_LIBRARY_EXTENSION " failed", "opengl32" FF_LIBRARY_EXTENSION, 1);

    WGLData data = { .result = result };

    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(opengl32, data, wglMakeCurrent);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(opengl32, data, wglCreateContext);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(opengl32, data, wglDeleteContext);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(opengl32, data, glGetString);

    MSG msg = {0};
    WNDCLASSW wc = {
        .lpfnWndProc = wglHandleWndProc,
        .hInstance = NULL,
        .hbrBackground = (HBRUSH)COLOR_BACKGROUND,
        .lpszClassName = L"ogl_version_check",
        .style = CS_OWNDC,
    };
    if(!RegisterClassW(&wc))
        return "RegisterClassW() failed";

    HWND hWnd = CreateWindowW(wc.lpszClassName, L"ogl_version_check", 0, 0, 0, FF_OPENGL_BUFFER_WIDTH, FF_OPENGL_BUFFER_HEIGHT, NULL, NULL, NULL, &data);

    while(GetMessageW(&msg, hWnd, 0, 0) > 0)
        DispatchMessage(&msg);

    return data.error;
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
