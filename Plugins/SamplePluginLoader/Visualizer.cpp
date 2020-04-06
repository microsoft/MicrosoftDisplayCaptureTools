#include "pch.h"
#include "Visualizer.h"
using namespace Windows::Foundation;

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)

const WCHAR g_szClassName[] = L"myWindowClass";

std::shared_ptr<PeekWindow> PeekWindow::GetOrCreateVisualizer()
{
    return PeekWindow::Instance();
}

void PeekWindow::InitializeWindow()
{
    m_windowThread = std::make_unique<std::thread>([&]
        {
            while (!m_startThread) {};

            if (BuildWindow())
            {
                std::error_code ec(E_FAIL, std::system_category());
                throw std::system_error(ec, "Failed Setup");
            }

            MSG Msg;

            while (GetMessage(&Msg, NULL, 0, 0) > 0)
            {
                TranslateMessage(&Msg);
                DispatchMessage(&Msg);

                if (m_forceExit) PostMessage(m_hwnd, WM_CLOSE, 0, 0);
            }

            m_kill = true;
        });

    m_startThread = true;
}

bool PeekWindow::CanExit()
{
    return m_kill;
}

PeekWindow::~PeekWindow()
{
    m_forceExit = true;
    if (m_windowThread) m_windowThread->join();
}

void PeekWindow::OnRender()
{
    CreateDeviceDependentResources();

    if (m_d2dRenderTarget)
    {
        m_d2dRenderTarget->BeginDraw();
        m_d2dRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));

        if (m_d2dBitmap)
        {
            std::unique_lock<std::mutex> lock(m_bitmapLock);

            auto imageSize = m_d2dBitmap->GetSize();
            auto imagePos = D2D1::Point2F((m_width - imageSize.width)/2, (m_height-imageSize.height)/2);
            auto imageRect = D2D1::RectF(
                imagePos.x, imagePos.y,
                imagePos.x + imageSize.width,
                imagePos.y + imageSize.height);

            float verticalScale = m_height / imageSize.height;
            float horizontalScale = m_width / imageSize.width;

            verticalScale   = min(verticalScale, horizontalScale);
            horizontalScale = min(verticalScale, horizontalScale);

            m_d2dRenderTarget->SetTransform(D2D1::Matrix3x2F::Scale(D2D1::SizeF(horizontalScale, verticalScale),D2D1::Point2F(m_width/2, m_height/2)));

            m_d2dRenderTarget->DrawBitmap(m_d2dBitmap.get(), imageRect);
        }

        if (m_d2dRenderTarget->EndDraw() == D2DERR_RECREATE_TARGET)
        {
            m_d2dRenderTarget->Release();
            m_d2dRenderTarget = nullptr;
        }
    }
}

void PeekWindow::OnResize(uint32_t width, uint32_t height)
{
    m_width = width;
    m_height = height;

    if (m_d2dRenderTarget)
    {
        m_d2dRenderTarget->Resize(D2D1::SizeU(width, height));
    }
}

void PeekWindow::CreateDeviceIndependentResources()
{
    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, m_d2dFactory.put());
}

void PeekWindow::CreateDeviceDependentResources()
{
    if (!m_d2dRenderTarget)
    {
        RECT rect;
        GetClientRect(m_hwnd, &rect);

        auto size = D2D1::SizeU(
            rect.right - rect.left,
            rect.bottom - rect.top
        );

        auto renderTargetProperties = D2D1::RenderTargetProperties();
        auto hwndRenderTargetProperties = D2D1::HwndRenderTargetProperties(m_hwnd, size);

        m_d2dFactory->CreateHwndRenderTarget(
            &renderTargetProperties,
            &hwndRenderTargetProperties,
            m_d2dRenderTarget.put()
        );
    }
}

void PeekWindow::UpdateDisplay(IWICBitmapSource* bitmapSource)
{
    if (m_d2dRenderTarget)
    {
        std::unique_lock<std::mutex> lock(m_bitmapLock);

        if (m_d2dBitmap)
        {
            m_d2dBitmap = nullptr;
        }

        m_d2dRenderTarget->CreateBitmapFromWicBitmap(bitmapSource, m_d2dBitmap.put());
    }

    InvalidateRect(m_hwnd, NULL, FALSE);
    UpdateWindow(m_hwnd);
}

LRESULT CALLBACK PeekWindow::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    std::shared_ptr<PeekWindow> visualizer = GetOrCreateVisualizer();

    switch (msg)
    {
    case WM_CLOSE:
        DestroyWindow(hwnd);
        visualizer->m_kill = true;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) PostQuitMessage(0);
        break;
    case WM_PAINT:
        if (visualizer) visualizer->OnRender();
        ValidateRect(hwnd, NULL);
        break;
    case WM_SIZE:
        if (visualizer) visualizer->OnResize(LOWORD(lParam), HIWORD(lParam));
        break;
    case WM_DISPLAYCHANGE:
        InvalidateRect(hwnd, NULL, FALSE);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int PeekWindow::BuildWindow()
{
    CreateDeviceIndependentResources();

    WNDCLASSEX wc;

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0;
    wc.lpfnWndProc = PeekWindow::WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = HINST_THISCOMPONENT;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = g_szClassName;
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wc))
    {
        return -1;
    }

    // Step 2: Creating the Window
    m_hwnd = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        g_szClassName,
        L"HWHLK Visualizer",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720,
        NULL, NULL, HINST_THISCOMPONENT, NULL);

    if (m_hwnd == NULL)
    {
        return -1;
    }

    ShowWindow(m_hwnd, true);
    UpdateWindow(m_hwnd);

    return 0;
}

Compositor::Compositor()
{
    
}

Compositor::~Compositor()
{

}