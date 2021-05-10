#ifndef _ANALOGCLOCK_WINDOW_H_
#define _ANALOGCLOCK_WINDOW_H_
#include "Window.h"

typedef struct AnalogClockWindow AnalogClockWindow;

typedef struct AnalogClockWindowVtbl
{
	void (*AddRef)(AnalogClockWindow*);
	unsigned int(*Release)(AnalogClockWindow*);
} AnalogClockWindowVtbl;

typedef struct AnalogClockWindow
{
	Window* Base;
	AnalogClockWindowVtbl* lpVtbl;

	D2D1_ELLIPSE clockFace;
	ID2D1SolidColorBrush* clockFaceBrush;
	ID2D1SolidColorBrush* clockContourBrush;
	HANDLE hClockRefreshThread;
	HANDLE hStopClockRefreshEvent;
} AnalogClockWindow;

AnalogClockWindow* _AnalogClockWindow(
	HINSTANCE hInstance,
	TCHAR* wszClassName,
	TCHAR* wszWindowName,
	ID2D1Factory1* d2d1Factory1
);
#endif