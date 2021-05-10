#ifndef _COMMON_H_
#define _COMMON_H_
#pragma warning(disable: 4995)
#include <stdlib.h>
#include <crtdbg.h>
#include <initguid.h>
#include <Windows.h>
#include <conio.h>
#include <tchar.h>
#include <stdio.h>
#include "d2d1_1.h"
#include <d3d11_1.h>
#pragma comment(lib, "d2d1")
#pragma comment(lib, "d3d11")

static unsigned int line = 0;
#define error_printf printf
#define IFSUCCESS(p) if (SUCCEEDED(p)) line = __LINE__; if (SUCCEEDED(p))

#define new

static const wchar_t MAIN_WINDOW_CLASS_NAME[] = L"Analog Clock";

#define HARDEXITONALLOCFAIL(p) \
if (!p) { \
    error_printf("%s:%d: 0x%x - Unable to allocate memory.", \
    __FILE__, __LINE__, E_OUTOFMEMORY);\
    int x = _getch(); \
    exit(E_OUTOFMEMORY); \
}
#define HARDEXITONRELEASEFAIL(p) \
if (p) { \
    error_printf("%s:%d: 0x%x - Unable to release memory." \
    "This likely indicates a bug in the memory handling of the application.\n" \
    "The reference count of the object was %d.\n", \
     __FILE__, __LINE__, E_OUTOFMEMORY); \
    int x = _getch(); \
    exit(E_OUTOFMEMORY); \
}

DEFINE_GUID(__uuidof_ID2D1Factory,
    0x06152247,
    0x6f50, 0x465a, 0x92, 0x45,
    0x11, 0x8b, 0xfd, 0x3b, 0x60, 0x07
);
DEFINE_GUID(__uuidof_IDXGIDevice,
    0x54ec77fa,
    0x1377, 0x44e6, 0x8c, 0x32,
    0x88, 0xfd, 0x5f, 0x44, 0xc8, 0x4c
);
DEFINE_GUID(__uuidof_IDXGIFactory2,
    0x50c83a1c,
    0xe072, 0x4c48, 0x87, 0xb0,
    0x36, 0x30, 0xfa, 0x36, 0xa6, 0xd0
);
DEFINE_GUID(__uuidof_IDXGISurface,
    0xcafcb56c,
    0x6ac3, 0x4889, 0xbf, 0x47,
    0x9e, 0x23, 0xbb, 0xd2, 0x60, 0xec
);

// Driver types supported
static D3D_DRIVER_TYPE DriverTypes[] =
{
    D3D_DRIVER_TYPE_HARDWARE,
    D3D_DRIVER_TYPE_WARP,
    D3D_DRIVER_TYPE_REFERENCE,
};
static UINT NumDriverTypes = ARRAYSIZE(DriverTypes);

// Feature levels supported
static D3D_FEATURE_LEVEL FeatureLevels[] =
{
    D3D_FEATURE_LEVEL_11_0,
    D3D_FEATURE_LEVEL_10_1,
    D3D_FEATURE_LEVEL_10_0,
    D3D_FEATURE_LEVEL_9_1
};
static UINT NumFeatureLevels = ARRAYSIZE(FeatureLevels);

static D2D1_MATRIX_3X2_F MatIdentity = { 1.f, 0.f, 0.f, 1.f, 0.f, 0.f };

inline void ErrorDescription(HRESULT hr)
{
    if (FACILITY_WINDOWS == HRESULT_FACILITY(hr))
        hr = HRESULT_CODE(hr);
    TCHAR* szErrMsg;

    if (FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&szErrMsg, 0, NULL) != 0)
    {
        _tprintf(TEXT("%s\n"), szErrMsg);
        LocalFree(szErrMsg);
    }
    else
        _tprintf(TEXT("No error description is available.\n"), hr);
}

typedef struct Common Common;

typedef struct CommonVtbl
{
    void (*AddRef)(Common*);
    unsigned int(*Release)(Common*);
} CommonVtbl;

typedef struct Common
{
    CommonVtbl* lpVtbl;

    unsigned int RefCount;
} Common;

void CommonAddRef(Common* This);

BOOL CommonRelease(Common* This);

Common* _Common();

#endif