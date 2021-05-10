#include "Window.h"

LRESULT CALLBACK Window_Proc(
    HWND hWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
)
{
    HRESULT hr = S_OK;

    Window* self;
    if (uMsg == WM_CREATE)
    {
        CREATESTRUCT* pCreate = (CREATESTRUCT*)(lParam);
        self = (Window*)(pCreate->lpCreateParams);
        SetWindowLongPtr(
            hWnd,
            GWLP_USERDATA,
            (LONG_PTR)self
        );

        if (FAILED(hr))
        {
            SetLastError(hr);
            return -1;
        }
    }
    else
    {
        LONG_PTR ptr = GetWindowLongPtr(
            hWnd,
            GWLP_USERDATA
        );
        self = (Window*)(ptr);
    }
    switch (uMsg)
    {
    case WM_DESTROY:
    {
        PostQuitMessage(0);
        break;
    }
    case WM_SIZE:
    {
        if (self->gfx->target && wParam != SIZE_MINIMIZED)
        {
            IFSUCCESS(hr)
            {
                hr = (self->gfx)->lpVtbl->ResizeSwapChainBitmap(self->gfx);
            }
            IFSUCCESS(hr)
            {
                hr = (self->gfx)->lpVtbl->Render(self->gfx);
            }
            if (FAILED(hr))
            {
                SetLastError(hr);
                PostQuitMessage(-1);
            }
        }
        if (self->lpVtbl->OnSize)
        {
            return self->lpVtbl->OnSize(self->CallerForOnSize, wParam, lParam);
        }
        break;
    }
    case WM_DISPLAYCHANGE:
    {
        IFSUCCESS(hr)
        {
            hr = (self->gfx)->lpVtbl->Render(self->gfx);
        }
        if (FAILED(hr))
        {
            SetLastError(hr);
            PostQuitMessage(-1);
        }
        break;
    }
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        IFSUCCESS(hr)
        {
            hr = (self->gfx)->lpVtbl->Render(self->gfx);
        }
        EndPaint(hWnd, &ps);
        if (FAILED(hr))
        {
            SetLastError(hr);
            PostQuitMessage(-1);
        }
        break;
    }
    return 0;

    }
    return DefWindowProc(
        hWnd,
        uMsg,
        wParam,
        lParam
    );
}

void Window_AddRef(Window* self)
{
    self->Base->lpVtbl->AddRef(self->Base);
}

unsigned int Window_Release(Window* self)
{
    unsigned int RefCount = self->Base->lpVtbl->Release(self->Base);
    if (!RefCount)
    {
        // Release members
        if (self->gfx)
        {
            self->gfx->lpVtbl->Release(self->gfx);
        }
        if (self->d2d1Factory1)
        {
            ID2D1Factory1_Release(self->d2d1Factory1);
        }
        DestroyCursor(self->hCursor);
        // Release virtual table
        if (self->lpVtbl)
        {
            free(self->lpVtbl);
        }
        // Release self
        if (self)
        {
            free(self);
        }
    }
    return RefCount;
}

Window* _Window(
    HINSTANCE hInstance, 
    TCHAR* wszClassName, 
    TCHAR* wszWindowName,
    ID2D1Factory1* d2d1Factory1
)
{
    // Allocate object
    Window* self = calloc(1, sizeof(Window));
    HARDEXITONALLOCFAIL(self);

    // Create base
    self->Base = new _Common();
    if (!self->Base)
    {
        return NULL;
    }

    // Allocate virtual table
    WindowVtbl* vtbl = calloc(1, sizeof(WindowVtbl));
    HARDEXITONALLOCFAIL(vtbl);

    // Populate virtual table
    vtbl->AddRef = Window_AddRef;
    vtbl->Release = Window_Release;
    vtbl->WindowProc = Window_Proc;

    // Assign virtual table to object
    self->lpVtbl = vtbl;

    // Populate members
    self->hCursor = LoadCursor(NULL, IDC_ARROW);
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = vtbl->WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = wszClassName;
    wc.hCursor = self->hCursor;
    if (!RegisterClass(&wc))
    {
        Window_Release(self);
        return NULL;
    }
    self->d2d1Factory1 = d2d1Factory1;
    ID2D1Factory1_AddRef(d2d1Factory1);
    self->hWnd = CreateWindowEx(
        0,
        wszClassName,
        wszWindowName,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL,
        NULL,
        hInstance,
        self
    );
    if (!self->hWnd)
    {
        Window_Release(self);
        return NULL;
    }
    self->gfx = new _WindowGraphics(
        d2d1Factory1,
        self->hWnd,
        &(self->lpVtbl->OnPaint),
        &(self->CallerForOnPaint)
    );
    return self;
}
