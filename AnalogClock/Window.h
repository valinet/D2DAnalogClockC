#ifndef _WINDOW_BASE_H_
#define _WINDOW_BASE_H_
#include "common.h"
#include "WindowGraphics.h"

typedef struct Window Window;

typedef struct WindowVtbl
{
    void (*AddRef)(Window*);
    unsigned int (*Release)(Window*);
    LRESULT (*WindowProc)(
        HWND,
        UINT,
        WPARAM,
        LPARAM
        );

    void (*OnPaint)(void*, ID2D1DeviceContext*);
    LRESULT (*OnSize)(void*, WPARAM, LPARAM);
} WindowVtbl;

typedef struct Window
{
    Common* Base;
    WindowVtbl* lpVtbl;

    void* CallerForOnPaint;
    void* CallerForOnSize;

    ID2D1Factory1* d2d1Factory1;
    HWND hWnd;
    WindowGraphics* gfx;
    HCURSOR hCursor;
} Window;

Window* _Window(
    HINSTANCE hInstance,
    TCHAR* wszClassName,
    TCHAR* wszWindowName,
    ID2D1Factory1* d2d1Factory1
);
#endif