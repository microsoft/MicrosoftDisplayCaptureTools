#pragma once
#include "Singleton.h"
#include <atomic>
#include <thread>

#include <d2d1.h>
#include <d2d1helper.h>

//
// Compositor
//      A class to compose multiple SoftwareBitmaps into a single displayable HBitmap
//
class Compositor
{
public:
    Compositor();
    ~Compositor();

private:

};

//
// PeekWindow
//      A class to manage a window for watching HWHLK plugin outputs in real time.
//
class PeekWindow : public Singleton<PeekWindow>, std::enable_shared_from_this<PeekWindow>
{
public:
    PeekWindow() {};
	~PeekWindow();
	static std::shared_ptr<PeekWindow> GetOrCreateVisualizer();

    void InitializeWindow();

    bool CanExit();

    void UpdateDisplay(IWICBitmapSource* bitmapSource);

private:
    static PeekWindow* singleton;

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    int BuildWindow();

    void OnRender();
    void OnResize(uint32_t width, uint32_t height);

    void CreateDeviceIndependentResources();
    void CreateDeviceDependentResources();

    winrt::com_ptr<ID2D1Factory> m_d2dFactory;
    winrt::com_ptr<ID2D1HwndRenderTarget> m_d2dRenderTarget;

    winrt::com_ptr<ID2D1Bitmap> m_d2dBitmap = nullptr;
    std::mutex m_bitmapLock;

    HWND m_hwnd = NULL;
    std::atomic<bool> m_forceExit = false;
    std::atomic<bool> m_startThread = false;
    std::atomic<bool> m_kill = false;
    std::unique_ptr<std::thread> m_windowThread;
    uint32_t m_width=0, m_height=0;
};