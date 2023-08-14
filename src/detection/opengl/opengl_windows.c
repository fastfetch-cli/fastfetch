#include "fastfetch.h"
#include "common/printing.h"
#include "opengl.h"

#include <Windows.h>
#include <GL/gl.h>
#ifndef GL_SHADING_LANGUAGE_VERSION // For WGL
    #define GL_SHADING_LANGUAGE_VERSION 0x8B8C
#endif

static const char* glHandleResult(FFOpenGLResult* result)
{
    ffStrbufAppendS(&result->version, (const char*) glGetString(GL_VERSION));
    ffStrbufAppendS(&result->renderer, (const char*) glGetString(GL_RENDERER));
    ffStrbufAppendS(&result->vendor, (const char*) glGetString(GL_VENDOR));
    ffStrbufAppendS(&result->slv, (const char*) glGetString(GL_SHADING_LANGUAGE_VERSION));
    return NULL;
}

const char* wglHandleContext(FFOpenGLResult* result, HDC hdc, HGLRC context)
{
    if(wglMakeCurrent(hdc, context) == FALSE)
        return "wglMakeCurrent() failed";
    return glHandleResult(result);
}

const char* wglHandlePixelFormat(FFOpenGLResult* result, HWND hWnd)
{
    PIXELFORMATDESCRIPTOR pfd =
    {
        sizeof(PIXELFORMATDESCRIPTOR),
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

    HGLRC context = wglCreateContext(hdc);
    if(context == NULL)
        return "wglCreateContext() failed";

    const char* error = wglHandleContext(result, hdc, context);
    wglDeleteContext(context);

    return error;
}

typedef struct WGLData
{
    FFOpenGLResult* result;
    const char* error;
} WGLData;

LRESULT CALLBACK wglHandleWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
    case WM_CREATE: {
        WGLData* wglData = (WGLData*)((CREATESTRUCT*)lParam)->lpCreateParams;
        wglData->error = wglHandlePixelFormat(wglData->result, hWnd);
        PostQuitMessage(0);
        return 0;
    }
    default:
        return DefWindowProcW(hWnd, message, wParam, lParam);
    }
}

const char* ffDetectOpenGL(FFOpenGLResult* result)
{
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

    WGLData data = { .result = result };
    HWND hWnd = CreateWindowW(wc.lpszClassName, L"ogl_version_check", 0, 0, 0, 1, 1, NULL, NULL, NULL, &data);

    while(GetMessageW(&msg, hWnd, 0, 0) > 0)
        DispatchMessage(&msg);

    return data.error;
}
