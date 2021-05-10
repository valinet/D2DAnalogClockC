#include "WindowGraphics.h"

HRESULT WindowGraphics_ReleaseDevice(WindowGraphics* self)
{
    HRESULT hr = S_OK;
    ULONG RefCountInner = 0;

    if (self->pBlackBrush)
    {
        RefCountInner = ID2D1DeviceContext_Release(self->pBlackBrush);
        self->pBlackBrush = NULL;
        HARDEXITONRELEASEFAIL(RefCountInner);
    }
    if (self->target)
    {
        RefCountInner = ID2D1DeviceContext_Release(self->target);
        self->target = NULL;
        HARDEXITONRELEASEFAIL(RefCountInner);
    }
    if (self->dxSwapChain1)
    {
        RefCountInner = self->dxSwapChain1->lpVtbl->Release(self->dxSwapChain1);
        self->dxSwapChain1 = NULL;
        HARDEXITONRELEASEFAIL(RefCountInner);
    }
    if (self->d3dDevice)
    {
        self->d3dDevice->lpVtbl->Release(self->d3dDevice);
        self->d3dDevice = NULL;
        HARDEXITONRELEASEFAIL(RefCountInner);
    }
    return hr;
}

HRESULT WindowGraphics_CreateDeviceSwapChainBitmap(WindowGraphics* self)
{
    HRESULT hr = S_OK;
    IDXGISurface* dxSurface = NULL;
    ID2D1Bitmap1* d2d1Bitmap1 = NULL;

    IFSUCCESS(hr)
    {
        hr = self->dxSwapChain1->lpVtbl->GetBuffer(
            self->dxSwapChain1,
            0,
            &__uuidof_IDXGISurface,
            &(dxSurface)
        );
    }
    IFSUCCESS(hr)
    {
        D2D1_PIXEL_FORMAT pixelFormat = { 0 };
        pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
        pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;

        D2D1_BITMAP_PROPERTIES1 props = { 0 };
        props.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
        props.colorContext = NULL;
        props.dpiX = 96.0f;
        props.dpiY = 96.0f;
        props.pixelFormat = pixelFormat;

        hr = self->target->lpVtbl->CreateBitmapFromDxgiSurface(
            self->target,
            dxSurface,
            &props,
            &(d2d1Bitmap1)
        );
        self->target->lpVtbl->SetTarget(
            self->target,
            d2d1Bitmap1
        );
        UINT dpi = GetDpiForWindow(self->hWnd);
        ID2D1DeviceContext_SetDpi(
            self->target,
            dpi,
            dpi
        );
    }

    if (dxSurface) dxSurface->lpVtbl->Release(dxSurface);
    if (d2d1Bitmap1) ID2D1Bitmap1_Release(d2d1Bitmap1);

    return hr;
}

HRESULT WindowGraphics_Render(WindowGraphics* self)
{
    HRESULT hr = S_OK, hrE = S_FALSE;

    if (self->target == NULL)
    {
        IFSUCCESS(hr)
        {
            hr = WindowGraphics_CreateDevice(self);
        }
        IFSUCCESS(hr)
        {
            hr = WindowGraphics_CreateDeviceSwapChainBitmap(self);
        }
        IFSUCCESS(hr)
        {
            if (self->lpVtbl->OnGfxRegenerate && self->CallerForOnGfxRegenerate)
            {
                hr = self->lpVtbl->OnGfxRegenerate(self->CallerForOnGfxRegenerate);
            }
        }
    }
    IFSUCCESS(hr)
    {
        ID2D1DeviceContext_BeginDraw(self->target);

        
        if (*(self->OnPaint) && *(self->CallerForOnPaint))
        {
            (*(self->OnPaint))(*(self->CallerForOnPaint), self->target);
        }

        D2D1_TAG tag1, tag2;
        hr = ID2D1DeviceContext_EndDraw(
            self->target,
            &tag1,
            &tag2
        );
    }
    if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET)
    {
        hrE = WindowGraphics_ReleaseDevice(self);
        if (FAILED(hrE))
        {
            hr = hrE;
        }
    }
    IFSUCCESS(hr)
    {
        hr = self->dxSwapChain1->lpVtbl->Present(
            self->dxSwapChain1,
            1,
            0
        );
    }
    if (FAILED(hr) && hr != DXGI_STATUS_OCCLUDED)
    {
        hrE = WindowGraphics_ReleaseDevice(self);
        if (FAILED(hrE))
        {
            hr = hrE;
        }
    }
    if (FAILED(hr) && FAILED(hrE))
    {
        return hrE != S_FALSE ? hrE : hr;
    }
    return S_OK;
}

HRESULT WindowGraphics_ResizeSwapChainBitmap(WindowGraphics* self)
{
    HRESULT hr = S_OK;

    self->target->lpVtbl->SetTarget(
        self->target,
        NULL
    );
    IFSUCCESS(hr)
    {
        hr = self->dxSwapChain1->lpVtbl->ResizeBuffers(
            self->dxSwapChain1,
            0,
            0,
            0,
            DXGI_FORMAT_UNKNOWN,
            0
        );
    }
    IFSUCCESS(hr)
    {
        hr = WindowGraphics_CreateDeviceSwapChainBitmap(self);
    }

    return hr;
}

HRESULT WindowGraphics_CreateDevice(WindowGraphics* self)
{
    HRESULT hr = S_OK;
    IDXGIDevice* dxDevice = NULL;
    IDXGIAdapter* dxAdapter = NULL;
    IDXGIFactory2* dxFactory2 = NULL;
    ID2D1Device* d2d1Device = NULL;

    IFSUCCESS(hr)
    {
        for (UINT DriverTypeIndex = 0; DriverTypeIndex < NumDriverTypes; ++DriverTypeIndex)
        {
            UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
            flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
            hr = D3D11CreateDevice(
                NULL,
                DriverTypes[DriverTypeIndex],
                NULL,
                flags,
                FeatureLevels,
                NumFeatureLevels,
                D3D11_SDK_VERSION,
                &(self->d3dDevice),
                NULL,
                NULL
            );
            if (SUCCEEDED(hr))
            {
                break;
            }
        }
    }
    IFSUCCESS(hr)
    {
        hr = self->d3dDevice->lpVtbl->QueryInterface(
            self->d3dDevice,
            &__uuidof_IDXGIDevice,
            &(dxDevice)
        );
    }
    IFSUCCESS(hr)
    {
        hr = dxDevice->lpVtbl->GetAdapter(
            dxDevice,
            &(dxAdapter)
        );
    }
    IFSUCCESS(hr)
    {
        hr = dxAdapter->lpVtbl->GetParent(
            dxAdapter,
            &__uuidof_IDXGIFactory2,
            &(dxFactory2)
        );
    }
    IFSUCCESS(hr)
    {
        DXGI_SWAP_CHAIN_DESC1 props = { 0 };
        ZeroMemory(&props, sizeof(DXGI_SWAP_CHAIN_DESC1));
        props.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        props.SampleDesc.Count = 1;
        props.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        props.BufferCount = 2;
        props.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
        hr = dxFactory2->lpVtbl->CreateSwapChainForHwnd(
            dxFactory2,
            self->d3dDevice,
            self->hWnd,
            &props,
            NULL,
            NULL,
            &(self->dxSwapChain1)
        );
    }
    IFSUCCESS(hr)
    {
        hr = self->d2d1Factory1->lpVtbl->CreateDevice(
            self->d2d1Factory1,
            dxDevice,
            &(d2d1Device)
        );
    }
    IFSUCCESS(hr)
    {
        hr = d2d1Device->lpVtbl->CreateDeviceContext(
            d2d1Device,
            D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
            &(self->target)
        );
    }
    IFSUCCESS(hr)
    {
        D3DCOLORVALUE color;
        color.a = 1;
        color.r = 0;
        color.g = 0;
        color.b = 0;

        D2D1_BRUSH_PROPERTIES props = { 0 };
        props.opacity = 1;
        props.transform = MatIdentity;
        hr = ID2D1DeviceContext_CreateSolidColorBrush(
            self->target,
            &color,
            &props,
            &(self->pBlackBrush)
        );
    }

    if (dxDevice) dxDevice->lpVtbl->Release(dxDevice);
    if (dxAdapter) dxAdapter->lpVtbl->Release(dxAdapter);
    if (dxFactory2) dxFactory2->lpVtbl->Release(dxFactory2);
    if (d2d1Device) ID2D1Device_Release(d2d1Device);

    return hr;
}

void WindowGraphics_AddRef(WindowGraphics* self)
{
    self->Base->lpVtbl->AddRef(self->Base);
}

unsigned int WindowGraphics_Release(WindowGraphics* self)
{
    unsigned int RefCount = self->Base->lpVtbl->Release(self->Base);
    if (!RefCount)
    {
        // Release members
        WindowGraphics_ReleaseDevice(self);
        // Release virtual table
        free(self->lpVtbl);
        // Release self
        free(self);
    }
    return RefCount;
}

WindowGraphics* _WindowGraphics(
    ID2D1Factory1* d2d1Factory, 
    HWND hWnd,
    void (**OnPaint)(ID2D1DeviceContext*),
    void** CallerForOnPaint
)
{
    // Allocate object
    WindowGraphics* self = calloc(1, sizeof(WindowGraphics));
    HARDEXITONALLOCFAIL(self);

    // Create base
    self->Base = new _Common();
    if (!self->Base)
    {
        return NULL;
    }

    // Allocate virtual table
    WindowGraphicsVtbl* vtbl = calloc(1, sizeof(WindowGraphicsVtbl));
    HARDEXITONALLOCFAIL(vtbl);

    // Populate virtual table
    vtbl->AddRef = WindowGraphics_AddRef;
    vtbl->Release = WindowGraphics_Release;
    vtbl->CreateDevice = WindowGraphics_CreateDevice;
    vtbl->CreateDeviceSwapChainBitmap = WindowGraphics_CreateDeviceSwapChainBitmap;
    vtbl->ReleaseDevice = WindowGraphics_ReleaseDevice;
    vtbl->Render = WindowGraphics_Render;
    vtbl->ResizeSwapChainBitmap = WindowGraphics_ResizeSwapChainBitmap;

    // Assign virtual table to object
    self->lpVtbl = vtbl;

    // Populate members
    self->d2d1Factory1 = d2d1Factory;
    self->hWnd = hWnd;
    self->OnPaint = OnPaint;
    self->CallerForOnPaint = CallerForOnPaint;

    return self;
}