#define _CRTDBG_MAP_ALLOC
#include "common.h"
#include "AnalogClockWindow.h"

int WINAPI wWinMain(
    HINSTANCE hInstance, 
    HINSTANCE hPrevInstance, 
    PWSTR pCmdLine, 
    int nCmdShow
)
{
    HRESULT hr = S_OK;
    int line = 0;
#ifdef _DEBUG
    FILE* conout = NULL;
    AllocConsole();
    freopen_s(
        &conout,
        "CONOUT$",
        "w",
        stdout
    );
#endif
    SetProcessDpiAwarenessContext(
        DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
    );

    ID2D1Factory1* m_d2d1Factory1 = NULL;
    AnalogClockWindow* analogClockWindow = NULL;

    IFSUCCESS(hr)
    {
        D2D1_FACTORY_OPTIONS fo = { 0 };
#ifdef _DEBUG
        fo.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#else
        fo.debugLevel = D2D1_DEBUG_LEVEL_NONE;
#endif
        hr = D2D1CreateFactory(
            D2D1_FACTORY_TYPE_SINGLE_THREADED,
            &__uuidof_ID2D1Factory,
            &fo,
            &m_d2d1Factory1
        );
    }
    IFSUCCESS(hr)
    {
        analogClockWindow = new _AnalogClockWindow(
            hInstance, 
            MAIN_WINDOW_CLASS_NAME,
            MAIN_WINDOW_CLASS_NAME,
            m_d2d1Factory1
        );
        hr = analogClockWindow ? S_OK : E_OUTOFMEMORY;
    }
    IFSUCCESS(hr)
    {
        ShowWindow(analogClockWindow->Base->hWnd, nCmdShow);
        BOOL bRet;
        MSG msg = { 0 };
        while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0)
        {
            if (bRet == -1)
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                break;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        (analogClockWindow)->lpVtbl->Release(analogClockWindow);
        ID2D1Factory1_Release(m_d2d1Factory1);
    }

    _CrtDumpMemoryLeaks();
    if (FAILED(hr))
    {
        error_printf(
            "%s:%d: 0x%x - ",
            __FILE__,
            line,
            hr
        );
        ErrorDescription(hr);
        _getch();
    }
    return hr;
}