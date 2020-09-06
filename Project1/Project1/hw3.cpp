#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable:4996) 

#include "hw3.h"

#pragma region WinMain
int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow
    )
{

    // Ignore the return value because we want to run the program even in the
    // unlikely event that HeapSetInformation fails.
    HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);
    if (SUCCEEDED(CoInitialize(NULL)))
    {
        {
            DemoApp app;

			//hInstance를 저장한다
			app.SetInstanceHandle(hInstance);

            if (SUCCEEDED(app.Initialize()))
            {
                app.RunMessageLoop();
            }
        }
        CoUninitialize();
    }

    return 0;
}
#pragma endregion

#pragma region DemoApp constructor     
DemoApp::DemoApp() :
	m_hwnd(NULL),
	m_pD2DFactory(NULL),
	m_pRenderTarget(NULL),
	m_pDWriteFactory(NULL),
	m_pWICFactory(NULL)
{
}
#pragma endregion

#pragma region ~DemoApp destructor     
DemoApp::~DemoApp()
{
	DiscardDeviceResources();
}
#pragma endregion


#pragma region SetInstanceHandle
void DemoApp::SetInstanceHandle(HINSTANCE hInstance) 
{ 
	m_hInst = hInstance; 
}
#pragma endregion

#pragma region Initialize
HRESULT DemoApp::Initialize()
{
    HRESULT hr;

    // Initialize device-indpendent resources, such
    // as the Direct2D factory.
    hr = CreateDeviceIndependentResources();
    if (SUCCEEDED(hr))
    {
        // Register the window class.
        WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
        wcex.style         = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc   = DemoApp::WndProc;
        wcex.cbClsExtra    = 0;
        wcex.cbWndExtra    = sizeof(LONG_PTR);
        wcex.hInstance     = m_hInst;
        wcex.hbrBackground = NULL;
        wcex.lpszMenuName  = NULL;
        wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
        wcex.lpszClassName = WINDOWPARAMETER_CLASSNAME;

        RegisterClassEx(&wcex);

        // Create the application window.
        m_hwnd = CreateWindow(
            WINDOWPARAMETER_CLASSNAME,
			WINDOWPARAMETER_WINDOWNAME,
            WS_OVERLAPPEDWINDOW,
			WINDOWPARAMETER_X, 
			WINDOWPARAMETER_X,
			WINDOWPARAMETER_W,
			WINDOWPARAMETER_H,
            NULL,
            NULL,
            m_hInst,
            this
            );

        hr = m_hwnd ? S_OK : E_FAIL;
        if (SUCCEEDED(hr))
        {
			//여기에 작성, 윈도우 생성시 해야할 처리
            ShowWindow(m_hwnd, SW_SHOWNORMAL);
            UpdateWindow(m_hwnd);
        }
    }

	m_gameManager.OnInitialize(m_pD2DFactory, m_pRenderTarget, m_pDWriteFactory, m_pWICFactory, m_hwnd);

    return hr;
}
#pragma endregion

#pragma region CreateDeviceIndependentResources
HRESULT DemoApp::CreateDeviceIndependentResources()
{
    HRESULT hr = S_OK;

    // Create D2D factory
    hr = D2D1CreateFactory(
        D2D1_FACTORY_TYPE_SINGLE_THREADED,
        &m_pD2DFactory
        );

    if (SUCCEEDED(hr))
    {
        // Create a shared DirectWrite factory
        hr = DWriteCreateFactory(
           DWRITE_FACTORY_TYPE_SHARED,
           __uuidof(IDWriteFactory),
           reinterpret_cast<IUnknown**>(&m_pDWriteFactory)
           );

		if (SUCCEEDED(hr))
		{
			hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_pWICFactory));
			if (SUCCEEDED(hr))
			{
			}
		}
    }

    return hr;
}
#pragma endregion

#pragma region CreateDeviceResources
HRESULT DemoApp::CreateDeviceResources()
{
    HRESULT hr = S_OK;

    if (!m_pRenderTarget)
    {
        RECT rc;
        GetClientRect(m_hwnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(
            rc.right - rc.left,
            rc.bottom - rc.top
            );

        // Create a Direct2D render target.
        hr = m_pD2DFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(m_hwnd, size),
            &m_pRenderTarget
            );
    }

    return hr;
}

#pragma endregion

#pragma region discard device resource
void DemoApp::DiscardDeviceResources()
{
	m_gameManager.Release();
	SafeRelease(m_pD2DFactory);
	SafeRelease(m_pRenderTarget);
	SafeRelease(m_pDWriteFactory);
	SafeRelease(m_pWICFactory);
}
#pragma endregion

#pragma region RunMessageLoop
void DemoApp::RunMessageLoop()
{
    MSG msg;

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}
#pragma endregion

#pragma region OnRender
HRESULT DemoApp::OnRender()
{
    HRESULT hr = CreateDeviceResources();

    if (SUCCEEDED(hr))
    {
		m_gameManager.OnUpdate();

		//그리기 시작한다
        m_pRenderTarget->BeginDraw();

        //변환행렬을 초기화한다
        m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

        //화면을 검은색으로 칠한다
        m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));

        D2D1_SIZE_F rtSize = m_pRenderTarget->GetSize();

		//OnRender를 호출한다
		m_gameManager.OnRender(m_pRenderTarget);

        //그리기 끝낸다
        m_pRenderTarget->EndDraw();		
    }

    if (hr == D2DERR_RECREATE_TARGET)
    {
        hr = S_OK;
        DiscardDeviceResources();
    }

    return hr;
}
#pragma endregion

#pragma region OnResize

void DemoApp::OnResize(UINT width, UINT height)
{
    if (m_pRenderTarget)
    {
        // Note: This method can fail, but it's okay to ignore the
        // error here -- the error will be repeated on the next call to
        // EndDraw.
        m_pRenderTarget->Resize(D2D1::SizeU(width, height));
    }
}

#pragma endregion

#pragma region WndProc
LRESULT CALLBACK DemoApp::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{

    if (message == WM_CREATE)
    {

        LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
        DemoApp *pDemoApp = (DemoApp *)pcs->lpCreateParams;

        ::SetWindowLongPtrW(
            hwnd,
            GWLP_USERDATA,
            PtrToUlong(pDemoApp)
            );

        return 1;
    }

    DemoApp *pDemoApp = reinterpret_cast<DemoApp *>(static_cast<LONG_PTR>(
        ::GetWindowLongPtrW(
            hwnd,
            GWLP_USERDATA
            )));

    if (pDemoApp)
    {
        switch (message)
        {
		case WM_CREATE:
			MoveWindow(hwnd, WINDOWPARAMETER_X, WINDOWPARAMETER_Y, WINDOWPARAMETER_W, WINDOWPARAMETER_H, TRUE);
			return 0;
		case WM_GETMINMAXINFO:
			((MINMAXINFO *)lParam)->ptMaxTrackSize.x = WINDOWPARAMETER_W;
			((MINMAXINFO *)lParam)->ptMaxTrackSize.y = WINDOWPARAMETER_H;
			((MINMAXINFO *)lParam)->ptMinTrackSize.x = WINDOWPARAMETER_W;
			((MINMAXINFO *)lParam)->ptMinTrackSize.y = WINDOWPARAMETER_H;
			return 0;
        case WM_SIZE:
            {
                UINT width = LOWORD(lParam);
                UINT height = HIWORD(lParam);
                pDemoApp->OnResize(width, height);
            }
            return 0;

        case WM_DISPLAYCHANGE:
            {
                InvalidateRect(hwnd, NULL, FALSE);
            }
            return 0;

        case WM_PAINT:
			pDemoApp->OnRender();
            return 0;

        case WM_DESTROY:
            {
                PostQuitMessage(0);
            }
            return 1;
        }
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}
#pragma endregion