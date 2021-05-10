#ifndef _WINDOW_GRAPHICS_H_
#define _WINDOW_GRAPHICS_H_
#include <Windows.h>
#include "d2d1_1.h"
#include <d3d11_1.h>
#include "common.h"

typedef struct WindowGraphics WindowGraphics;

typedef struct WindowGraphicsVtbl
{
    void (*AddRef)(WindowGraphics*);
    unsigned int(*Release)(WindowGraphics*);
    HRESULT (*ReleaseDevice)(WindowGraphics*);
    HRESULT (*CreateDeviceSwapChainBitmap)(WindowGraphics*);
    HRESULT (*Render)(WindowGraphics*);
    HRESULT (*ResizeSwapChainBitmap)(WindowGraphics*);
    HRESULT (*CreateDevice)(WindowGraphics*);
    HRESULT (*OnGfxRegenerate)(void*);
} WindowGraphicsVtbl;

typedef struct WindowGraphics
{
    Common* Base;
    WindowGraphicsVtbl* lpVtbl;

    HWND hWnd;
    ID2D1Factory1* d2d1Factory1;
    ID3D11Device* d3dDevice;
    IDXGISwapChain1* dxSwapChain1;
    ID2D1DeviceContext* target;
    ID2D1SolidColorBrush* pBlackBrush;
    void (**OnPaint)(void*, ID2D1DeviceContext*);
    void** CallerForOnPaint;
    void* CallerForOnGfxRegenerate;
} WindowGraphics;

WindowGraphics* _WindowGraphics(
    ID2D1Factory1*, 
    HWND,
    void (**)(void*, ID2D1DeviceContext*),
    void**
);

#endif