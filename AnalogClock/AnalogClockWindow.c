#include "AnalogClockWindow.h"

DWORD AnalogClockWindow_ClockRefreshThread(AnalogClockWindow* self)
{
    HWND hWnd = self->Base->hWnd;
    while (TRUE)
    {
        InvalidateRect(hWnd, NULL, FALSE);
        DWORD res = WaitForSingleObject(
            self->hStopClockRefreshEvent,
            1000 / 60
        );
        if (res == WAIT_OBJECT_0 + 0)
        {
            break;
        }
    }
}

void AnalogClockWindow_ClockRefreshTimerProc(
    HWND hWnd,
    UINT unnamedParam2,
    UINT_PTR unnamedParam3,
    DWORD unnamedParam4
)
{
    InvalidateRect(hWnd, NULL, FALSE);
}

void AnalogClockWindow_ReleaseResources(AnalogClockWindow* self)
{
    if (self->clockFaceBrush)
    {
        unsigned int RefCountInner = ID2D1DeviceContext_Release(self->clockFaceBrush);
        self->clockFaceBrush = NULL;
        HARDEXITONRELEASEFAIL(RefCountInner);
    }
    if (self->clockContourBrush)
    {
        unsigned int RefCountInner = ID2D1DeviceContext_Release(self->clockContourBrush);
        self->clockContourBrush = NULL;
        HARDEXITONRELEASEFAIL(RefCountInner);
    }
}

void AnalogClockWindow_CalculateLayout(AnalogClockWindow* self)
{
    if (self->Base->gfx->target)
    {
        void(*ID2D1DeviceContext_GetSizeFixed)(ID2D1DeviceContext*, D2D1_SIZE_F*) = (self->Base->gfx->target)->lpVtbl->Base.GetSize;
        D2D1_SIZE_F size;
        ID2D1DeviceContext_GetSizeFixed(self->Base->gfx->target, &size);
        D2D1_POINT_2F point;
        point.x = size.width / 2;
        point.y = size.height / 2;
        self->clockFace.point = point;
        self->clockFace.radiusX = min(point.x, point.y);
        self->clockFace.radiusY = self->clockFace.radiusX;
    }
}

HRESULT AnalogClockWindow_OnGfxRegenerate(AnalogClockWindow* self)
{
    HRESULT hr = S_OK;
    AnalogClockWindow_ReleaseResources(self);
    D3DCOLORVALUE color;
    color.a = 1;
    color.r = 1;
    color.g = 1;
    color.b = 0;
    D2D1_BRUSH_PROPERTIES props = { 0 };
    props.opacity = 1;
    props.transform = MatIdentity;
    hr = ID2D1DeviceContext_CreateSolidColorBrush(
        self->Base->gfx->target,
        &color,
        &props,
        &(self->clockFaceBrush)
    );
    color.a = 1;
    color.r = 0;
    color.g = 0;
    color.b = 0;
    hr = ID2D1DeviceContext_CreateSolidColorBrush(
        self->Base->gfx->target,
        &color,
        &props,
        &(self->clockContourBrush)
    );
    AnalogClockWindow_CalculateLayout(self);
    return hr;
}

LRESULT AnalogClockWindow_OnSize(
    AnalogClockWindow* self, 
    WPARAM wParam, 
    LPARAM lParam
)
{
    AnalogClockWindow_CalculateLayout(self);
    return 0;
}

void AnalogClockWindow_DrawClockHand(
    AnalogClockWindow* self, 
    ID2D1DeviceContext* target, 
    float fHandLength, 
    float fAngle, 
    float fStrokeWidth
)
{
    D2D1_MATRIX_3X2_F rotationMatrix;
    D2D1MakeRotateMatrix(
        fAngle,
        self->clockFace.point,
        &rotationMatrix
    );

    ID2D1DeviceContext_SetTransform(
        target,
        &rotationMatrix
    );

    D2D_POINT_2F endPoint;
    endPoint.x = self->clockFace.point.x;
    endPoint.y = self->clockFace.point.y - (self->clockFace.radiusY * fHandLength);

    ID2D1DeviceContext_DrawLine(
        target,
        self->clockFace.point,
        endPoint,
        self->clockContourBrush,
        fStrokeWidth,
        NULL
    );
}

void AnalogClockWindow_OnPaint(
    AnalogClockWindow* self, 
    ID2D1DeviceContext* target
)
{
    D3DCOLORVALUE color;
    color.a = 1;
    color.r = 0.521f;
    color.g = 0.976f;
    color.b = 0.960f;

    ID2D1DeviceContext_Clear(
        target,
        &color
    );
    ID2D1DeviceContext_FillEllipse(
        target,
        &(self->clockFace),
        self->clockFaceBrush
    );
    ID2D1DeviceContext_DrawEllipse(
        target,
        &(self->clockFace),
        self->clockContourBrush,
        1.0f,
        NULL
    );

    SYSTEMTIME time;
    GetLocalTime(&time);

    const float fHourAngle = (360.0f / 12) * (time.wHour) + (time.wMinute * 0.5f);
    const float fMinuteAngle = (360.0f / 60) * (time.wMinute);
    const float fSecondAngle =
        (360.0f / 60) * (time.wSecond) + (360.0f / 60000) * (time.wMilliseconds);

    AnalogClockWindow_DrawClockHand(
        self,
        target,
        0.6f, 
        fHourAngle, 
        6
    );
    AnalogClockWindow_DrawClockHand(
        self,
        target,
        0.85f, 
        fMinuteAngle, 
        4
    );
    AnalogClockWindow_DrawClockHand(
        self,
        target,
        0.85f,
        fSecondAngle,
        1
    );

    ID2D1DeviceContext_SetTransform(
        target,
        &MatIdentity
    );
}

void AnalogClockWindow_AddRef(AnalogClockWindow* self)
{
    self->Base->lpVtbl->AddRef(self->Base);
}

unsigned int AnalogClockWindow_Release(AnalogClockWindow* self)
{
    unsigned int RefCount = self->Base->lpVtbl->Release(self->Base);
    if (!RefCount)
    {
        // Release members
        AnalogClockWindow_ReleaseResources(self);
        SetEvent(self->hStopClockRefreshEvent);
        WaitForSingleObject(
            self->hClockRefreshThread,
            INFINITE
        );
        CloseHandle(self->hStopClockRefreshEvent);
        CloseHandle(self->hClockRefreshThread);
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

AnalogClockWindow* _AnalogClockWindow(
    HINSTANCE hInstance,
    TCHAR* wszClassName,
    TCHAR* wszWindowName,
    ID2D1Factory1* d2d1Factory1
)
{
    // Allocate object
    AnalogClockWindow* self = calloc(1, sizeof(AnalogClockWindow));
    HARDEXITONALLOCFAIL(self);

    // Create base
    self->Base = new _Window(
        hInstance,
        wszClassName,
        wszWindowName,
        d2d1Factory1
    );
    if (!self->Base)
    {
        return NULL;
    }

    // Allocate virtual table
    AnalogClockWindowVtbl* vtbl = calloc(1, sizeof(AnalogClockWindowVtbl));
    HARDEXITONALLOCFAIL(vtbl);

    // Populate virtual table
    vtbl->AddRef = AnalogClockWindow_AddRef;
    vtbl->Release = AnalogClockWindow_Release;

    // Assign virtual table to object
    self->lpVtbl = vtbl;

    // Populate members
    self->Base->CallerForOnPaint = self;
    self->Base->lpVtbl->OnPaint = AnalogClockWindow_OnPaint;

    self->Base->CallerForOnSize = self;
    self->Base->lpVtbl->OnSize = AnalogClockWindow_OnSize;

    self->Base->gfx->CallerForOnGfxRegenerate = self;
    self->Base->gfx->lpVtbl->OnGfxRegenerate = AnalogClockWindow_OnGfxRegenerate;

    self->hStopClockRefreshEvent = CreateEvent(
        NULL,
        FALSE,
        FALSE,
        NULL
    );
    self->hClockRefreshThread = CreateThread(
        NULL,
        NULL,
        AnalogClockWindow_ClockRefreshThread,
        self,
        NULL,
        NULL
    );
    /*
    SetTimer(
        self->Base->hWnd,
        0,
        1000 / 60,
        AnalogClockWindow_RefreshTimerProc
    );
    */
    return self;
}