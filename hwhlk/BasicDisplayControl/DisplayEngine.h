#pragma once
#include "DisplayEngine.g.h"

namespace MonitorUtilities
{
    class MonitorControl;
}

namespace winrt::DisplayControl::implementation
{
    constexpr double sc_refreshRateEpsilon = 0.00000000001;

    struct DisplayEnginePlanePropertySet : implements<DisplayEnginePlanePropertySet, winrt::MicrosoftDisplayCaptureTools::Display::IDisplayEnginePlanePropertySet>
    {
        DisplayEnginePlanePropertySet() = default;

        bool Active();
        void Active(bool active);        

        winrt::Windows::Graphics::Imaging::BitmapBounds Rect();
        void Rect(winrt::Windows::Graphics::Imaging::BitmapBounds bounds);

        Windows::Graphics::DirectX::DirectXPixelFormat Format();
        void Format(Windows::Graphics::DirectX::DirectXPixelFormat format);

        Windows::Graphics::Imaging::SoftwareBitmap SourceBitmap();
        void SourceBitmap(Windows::Graphics::Imaging::SoftwareBitmap bitmap);

        MicrosoftDisplayCaptureTools::Display::PixelColor ClearColor();
        void ClearColor(MicrosoftDisplayCaptureTools::Display::PixelColor pixelColor);

        bool m_active = false;
        MicrosoftDisplayCaptureTools::Display::PixelColor m_color {0};
    };

    struct DisplayEnginePlaneCapabilities : implements<DisplayEnginePlaneCapabilities, winrt::MicrosoftDisplayCaptureTools::Display::IDisplayEnginePlaneCapabilities>
    {
        DisplayEnginePlaneCapabilities() = default;

        // TODO: this is a placeholder for the time being
        winrt::hstring Name() { return L"Placeholder"; };
    };

    struct DisplayEnginePropertySet : implements<DisplayEnginePropertySet, winrt::MicrosoftDisplayCaptureTools::Display::IDisplayEnginePropertySet>
    {
        DisplayEnginePropertySet();

        winrt::Windows::Devices::Display::Core::DisplayModeInfo ActiveMode();
        void ActiveMode(winrt::Windows::Devices::Display::Core::DisplayModeInfo mode);

        double RefreshRate();
        void RefreshRate(double rate);

        winrt::Windows::Graphics::SizeInt32 Resolution();
        void Resolution(winrt::Windows::Graphics::SizeInt32 resolution);

        com_array<winrt::MicrosoftDisplayCaptureTools::Display::IDisplayEnginePlanePropertySet> GetPlaneProperties();

        double m_refreshRate;
        winrt::Windows::Graphics::SizeInt32 m_resolution;
        winrt::Windows::Devices::Display::Core::DisplayModeInfo m_mode;
        std::vector<winrt::com_ptr<DisplayEnginePlanePropertySet>> m_planeProperties;

        bool m_requeryMode;
        bool RequeryMode();
    };

    struct DisplayEngineCapabilities : implements<DisplayEngineCapabilities, winrt::MicrosoftDisplayCaptureTools::Display::IDisplayEngineCapabilities>
    {
        DisplayEngineCapabilities();
        com_array<winrt::Windows::Devices::Display::Core::DisplayModeInfo> GetSupportedModes();
        com_array<winrt::MicrosoftDisplayCaptureTools::Display::IDisplayEnginePlaneCapabilities> GetPlaneCapabilities();

        std::vector<winrt::com_ptr<DisplayEnginePlaneCapabilities>> m_planeCapabilities;
        std::vector<winrt::Windows::Devices::Display::Core::DisplayModeInfo> m_modes;
    };

    struct DisplayEnginePrediction : implements<DisplayEnginePrediction, winrt::MicrosoftDisplayCaptureTools::Display::IDisplayEnginePrediction>
    {
        DisplayEnginePrediction(DisplayEnginePropertySet* properties);

        winrt::Windows::Graphics::Imaging::SoftwareBitmap GetBitmap();

    private:
        winrt::Windows::Graphics::Imaging::SoftwareBitmap m_bitmap{ nullptr };
    };

    //
    // This renderer class handles the connection to a physical display through the GPU. This class represents a render loop
    // drawing to the target output. Note that this is an implementation detail - other DisplayEngine implementations are
    // welcome to use any means of getting pixels on screen - this uses D3D11 encapsulated in this 'Renderer'.
    //
    struct Renderer : implements<Renderer, winrt::Windows::Foundation::IClosable>
    {
    public: // Constructors/Destructors/IClosable
        Renderer() = default;

        ~Renderer() { Close(); };
        void Close();

    public: // Utility functions
        void RefreshMode();
        void StartRender(DisplayEnginePropertySet* properties);

    public: // Direct Display Objects
        winrt::Windows::Devices::Display::Core::DisplayManager displayManager{ nullptr };
        winrt::Windows::Devices::Display::Core::DisplayTarget displayTarget{ nullptr };
        winrt::Windows::Devices::Display::Core::DisplayState displayState{ nullptr };
        winrt::Windows::Devices::Display::Core::DisplayPath displayPath{ nullptr };
        winrt::Windows::Devices::Display::Core::DisplayDevice displayDevice{ nullptr };
        winrt::Windows::Devices::Display::Core::DisplayModeInfo displayMode{ nullptr };

    public: // D3D11 Objects
        winrt::com_ptr<ID3D11Device5> m_d3dDevice;
        winrt::com_ptr<ID3D11DeviceContext> m_d3dContext;
        winrt::com_ptr<ID3D11Texture2D> m_d3dSurface;
        winrt::com_ptr<ID3D11RenderTargetView> m_d3dRenderTarget;
        winrt::com_ptr<ID3D11Fence> m_d3dFence;
        uint64_t m_d3dFenceValue = 0;

    private: // Tool-set properties for this test rendering
        DisplayEnginePropertySet* m_properties{ nullptr };

    private: // Render thread handlers
        void Render();
        std::thread renderThread;
        std::atomic_bool m_valid = true;
        std::atomic_bool m_presenting = false;
    };

    struct DisplayEngine : DisplayEngineT<DisplayEngine>
    {
        DisplayEngine();

        ~DisplayEngine();

        hstring Name();
        void InitializeForDisplayTarget(winrt::Windows::Devices::Display::Core::DisplayTarget const& target);
        void InitializeForStableMonitorId(winrt::hstring target);
        winrt::Windows::Devices::Display::Core::DisplayTarget GetTarget();
        winrt::MicrosoftDisplayCaptureTools::Display::IDisplayEngineCapabilities GetCapabilities();
        winrt::MicrosoftDisplayCaptureTools::Display::IDisplayEnginePropertySet GetProperties();
        winrt::MicrosoftDisplayCaptureTools::Display::IDisplayEnginePrediction GetPrediction();
        void SetConfigData(winrt::Windows::Data::Json::IJsonValue data);
        winrt::Windows::Foundation::IClosable StartRender();

    private:
        void RefreshTarget();
        void ConnectTarget();
        void PopulateCapabilities();

    private:
        winrt::Windows::Devices::Display::Core::DisplayManager m_displayManager{ nullptr };
        winrt::Windows::Devices::Display::Core::DisplayTarget m_displayTarget{ nullptr };
        winrt::Windows::Devices::Display::Core::DisplayState m_displayState{ nullptr };
        winrt::Windows::Devices::Display::Core::DisplayPath m_displayPath{ nullptr };
        winrt::Windows::Devices::Display::Core::DisplayDevice m_displayDevice{ nullptr };

        winrt::com_ptr<DisplayEngineCapabilities> m_capabilities;
        winrt::com_ptr<DisplayEnginePropertySet> m_propertySet;

        std::mutex m_targetLock;
        std::unique_ptr<MonitorUtilities::MonitorControl> m_monitorControl;
    };
}
namespace winrt::DisplayControl::factory_implementation
{
    struct DisplayEngine : DisplayEngineT<DisplayEngine, implementation::DisplayEngine>
    {
    };
}