#pragma once
#pragma comment( lib , "WindowsCodecs.lib")

#ifndef UNICODE
#define UNICODE
#endif

// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <cmath>

#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>
#include <dwmapi.h>

#include "Spacecraft.h"
#include <list>

/******************************************************************
*                                                                 *
*  Macros                                                         *
*                                                                 *
******************************************************************/
#define WINDOWPARAMETER_CLASSNAME (L"test")
#define WINDOWPARAMETER_WINDOWNAME (L"test")
#define WINDOWPARAMETER_X (100)
#define WINDOWPARAMETER_Y (100)
#define WINDOWPARAMETER_W (400)
#define WINDOWPARAMETER_H (400)



#define SafeRelease(p) { if(p) { (p)->Release(); (p)=NULL; } }

#ifndef Assert
#if defined( DEBUG ) || defined( _DEBUG )
#define Assert(b) do {if (!(b)) {OutputDebugStringA("Assert: " #b "\n");}} while(0)
#else
#define Assert(b)
#endif //DEBUG || _DEBUG
#endif

/******************************************************************
*                                                                 *
*  DemoApp                                                        *
*                                                                 *
******************************************************************/
class DemoApp
{
public:
    DemoApp();
    ~DemoApp();

	// 초기화 전 Instance핸들 설정
	void SetInstanceHandle(HINSTANCE hInstance);

    // Register the window class and call methods for instantiating drawing resources
    HRESULT Initialize();

    // Process and dispatch messages
    void RunMessageLoop();
private:

    // Initialize device-independent resources
    HRESULT CreateDeviceIndependentResources();

    // Initialize device-dependent resources
    HRESULT CreateDeviceResources();

    // Release device-dependent resources
    void DiscardDeviceResources();

    // Draw content
    HRESULT OnRender();

    // Resize the render target
    void OnResize(
        UINT width,
        UINT height
        );
	
    // The windows procedure
    static LRESULT CALLBACK WndProc(
        HWND hWnd,
        UINT message,
        WPARAM wParam,
        LPARAM lParam
        );

private:
	HINSTANCE m_hInst;
    HWND m_hwnd;
    ID2D1Factory *m_pD2DFactory;
    ID2D1HwndRenderTarget *m_pRenderTarget;
    IDWriteFactory *m_pDWriteFactory;
	IWICImagingFactory *m_pWICFactory;

	GameManager m_gameManager;
};