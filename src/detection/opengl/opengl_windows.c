#include "opengl.h"
#include "common/debug.h"
#include "common/library.h"
#include "common/printing.h"
#include "common/windows/nt.h"

#include <windows.h>
#include <GL/gl.h>

typedef struct WGLData {
    FF_LIBRARY_SYMBOL(glGetString)
    FF_LIBRARY_SYMBOL(wglMakeCurrent)
    FF_LIBRARY_SYMBOL(wglCreateContext)
    FF_LIBRARY_SYMBOL(wglDeleteContext)
} WGLData;

void ffOpenGLHandleResult(FFOpenGLResult* result, typeof(&glGetString) ffglGetString);

static const char* wglHandleContext(WGLData* wglData, FFOpenGLResult* result, HDC hdc, HGLRC context) {
    if (wglData->ffwglMakeCurrent(hdc, context) == FALSE) {
        FF_DEBUG("wglMakeCurrent() failed: %s", ffDebugWin32Error(GetLastError()));
        return "wglMakeCurrent() failed";
    }
    ffOpenGLHandleResult(result, wglData->ffglGetString);
    ffStrbufSetStatic(&result->library, "WGL 1.0");
    if (wglData->ffwglMakeCurrent(nullptr, nullptr) == FALSE) {
        FF_DEBUG("wglMakeCurrent(nullptr, nullptr) failed: %s", ffDebugWin32Error(GetLastError()));
        return "wglMakeCurrent(nullptr, nullptr) failed";
    }
    return nullptr;
}

static const char* wglHandlePixelFormat(WGLData* wglData, FFOpenGLResult* result, HWND hWnd) {
    HDC hdc = GetDC(hWnd);

    if (hdc == nullptr) {
        FF_DEBUG("GetDC() failed: %s", ffDebugWin32Error(GetLastError()));
        return "GetDC() failed";
    }

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
    if (pixelFormat == 0) {
        FF_DEBUG("ChoosePixelFormat() failed: %s", ffDebugWin32Error(GetLastError()));
        ReleaseDC(hWnd, hdc);
        return "ChoosePixelFormat() failed";
    }

    if (SetPixelFormat(hdc, pixelFormat, &pfd) == FALSE) {
        FF_DEBUG("SetPixelFormat() failed: %s", ffDebugWin32Error(GetLastError()));
        ReleaseDC(hWnd, hdc);
        return "SetPixelFormat() failed";
    }

    HGLRC context = wglData->ffwglCreateContext(hdc);
    if (context == nullptr) {
        FF_DEBUG("wglCreateContext() failed: %s", ffDebugWin32Error(GetLastError()));
        ReleaseDC(hWnd, hdc);
        return "wglCreateContext() failed";
    }

    const char* error = wglHandleContext(wglData, result, hdc, context);
    wglData->ffwglDeleteContext(context);

    ReleaseDC(hWnd, hdc);

    return error;
}

static const char* wglDetectOpenGL(FFOpenGLResult* result) {
    FF_LIBRARY_LOAD_MESSAGE(opengl32, "opengl32" FF_LIBRARY_EXTENSION, 1);

    WGLData data = {};

    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(opengl32, data, wglMakeCurrent);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(opengl32, data, wglCreateContext);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(opengl32, data, wglDeleteContext);
    FF_LIBRARY_LOAD_SYMBOL_VAR_MESSAGE(opengl32, data, glGetString);

    HINSTANCE hInstance = ffGetPeb()->ImageBaseAddress;

    WNDCLASSW wc = {
        .lpfnWndProc = DefWindowProcW,
        .hInstance = hInstance,
        .hbrBackground = (HBRUSH) COLOR_BACKGROUND,
        .lpszClassName = L"ogl_version_check",
        .style = CS_OWNDC,
    };
    if (!RegisterClassW(&wc)) {
        FF_DEBUG("RegisterClassW() failed: %s", ffDebugWin32Error(GetLastError()));
        return "RegisterClassW() failed";
    }

    HWND hWnd = CreateWindowW(wc.lpszClassName, L"ogl_version_check", 0, 0, 0, FF_OPENGL_BUFFER_WIDTH, FF_OPENGL_BUFFER_HEIGHT, nullptr, nullptr, hInstance, nullptr);
    if (!hWnd) {
        FF_DEBUG("CreateWindowW() failed: %s", ffDebugWin32Error(GetLastError()));
        return "CreateWindowW() failed";
    }

    const char* error = wglHandlePixelFormat(&data, result, hWnd);

    DestroyWindow(hWnd);
    UnregisterClassW(wc.lpszClassName, hInstance);

    return error;
}

const char* ffDetectOpenGL(FFOpenGLOptions* options, FFOpenGLResult* result) {
    if (options->library == FF_OPENGL_LIBRARY_AUTO) {
        return wglDetectOpenGL(result);
    } else if (options->library == FF_OPENGL_LIBRARY_EGL) {
#if __has_include(<EGL/egl.h>)
        const char* ffOpenGLDetectByEGL(FFOpenGLResult * result);
        return ffOpenGLDetectByEGL(result);
#else
        FF_DEBUG("fastfetch was compiled without egl support");
        return "fastfetch was compiled without egl support";
#endif
    } else {
        FF_DEBUG("Unsupported OpenGL library");
        return "Unsupported OpenGL library";
    }
}
