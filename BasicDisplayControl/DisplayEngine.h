#pragma once
#include "DisplayEngine.g.h"
#include "DisplayEngineFactory.g.h"

namespace MonitorUtilities
{
    class MonitorControl;
}

namespace winrt::BasicDisplayControl::implementation
{
    constexpr double sc_refreshRateEpsilon = 0.00000000001;

    struct DisplayEnginePlanePropertySet : implements<DisplayEnginePlanePropertySet, MicrosoftDisplayCaptureTools::Display::IDisplayEnginePlanePropertySet>
    {
        DisplayEnginePlanePropertySet(MicrosoftDisplayCaptureTools::Framework::ILogger const& logger);

        ~DisplayEnginePlanePropertySet()
        {
        }

        // Required plane properties

        bool Active();
        void Active(bool active);        

        Windows::Graphics::Imaging::BitmapBounds Rect();
        void Rect(Windows::Graphics::Imaging::BitmapBounds bounds);

        Windows::Graphics::DirectX::DirectXPixelFormat Format();
        void Format(Windows::Graphics::DirectX::DirectXPixelFormat format);

        Windows::Foundation::Collections::IMap<hstring, IInspectable> PropertyBag();

    private:
        const MicrosoftDisplayCaptureTools::Framework::ILogger m_logger{nullptr};
        bool m_active = false;
        Windows::Graphics::Imaging::BitmapBounds m_rect{0};
        Windows::Graphics::DirectX::DirectXPixelFormat m_format{ Windows::Graphics::DirectX::DirectXPixelFormat::Unknown };
        Windows::Foundation::Collections::IMap<hstring, IInspectable> m_propertyBag;
    };

    struct DisplayEnginePropertySet : implements<DisplayEnginePropertySet, MicrosoftDisplayCaptureTools::Display::IDisplayEnginePropertySet>
    {
        DisplayEnginePropertySet(MicrosoftDisplayCaptureTools::Framework::ILogger const& logger);

        Windows::Devices::Display::Core::DisplayModeInfo ActiveMode();
        void ActiveMode(Windows::Devices::Display::Core::DisplayModeInfo mode);

        double RefreshRate();
        void RefreshRate(double rate);

        Windows::Graphics::SizeInt32 Resolution();
        void Resolution(Windows::Graphics::SizeInt32 resolution);

        com_array<MicrosoftDisplayCaptureTools::Display::IDisplayEnginePlanePropertySet> GetPlaneProperties();

        double m_refreshRate;
        Windows::Graphics::SizeInt32 m_resolution;
        Windows::Devices::Display::Core::DisplayModeInfo m_mode;
        std::vector<com_ptr<DisplayEnginePlanePropertySet>> m_planeProperties;

        bool m_requeryMode;
        bool RequeryMode();

    private:
        const MicrosoftDisplayCaptureTools::Framework::ILogger m_logger{nullptr};
    };

    struct DisplayEnginePrediction : implements<DisplayEnginePrediction, MicrosoftDisplayCaptureTools::Display::IDisplayEnginePrediction>
    {
        DisplayEnginePrediction(DisplayEnginePropertySet* properties, MicrosoftDisplayCaptureTools::Framework::ILogger const& logger);

        MicrosoftDisplayCaptureTools::Framework::IFrameData GetFrameData();

    private:
        MicrosoftDisplayCaptureTools::Framework::IFrameData m_frameData{nullptr};

        const MicrosoftDisplayCaptureTools::Framework::ILogger m_logger{nullptr};
    };

    //
    // This renderer class handles the connection to a physical display through the GPU. This class represents a render loop
    // drawing to the target output. Note that this is an implementation detail - other DisplayEngine implementations are
    // welcome to use any means of getting pixels on screen - this uses D3D11 encapsulated in this 'Renderer'.
    //
    struct DisplayOutput : implements<DisplayOutput, MicrosoftDisplayCaptureTools::Display::IDisplayOutput>
    {
        DisplayOutput(
            MicrosoftDisplayCaptureTools::Framework::ILogger const& logger, 
            Windows::Devices::Display::Core::DisplayTarget const& target,
            Windows::Devices::Display::Core::DisplayManager const& manager);

        ~DisplayOutput();

        Windows::Devices::Display::Core::DisplayTarget Target();
        MicrosoftDisplayCaptureTools::Display::IDisplayEnginePropertySet GetProperties();
        MicrosoftDisplayCaptureTools::Display::IDisplayEnginePrediction GetPrediction();

        Windows::Foundation::IClosable StartRender();
        void StopRender();

        event_token DisplaySetupCallback(Windows::Foundation::EventHandler<MicrosoftDisplayCaptureTools::Display::IDisplayEnginePropertySet> const& handler);
        void DisplaySetupCallback(event_token const& token) noexcept;

        event_token RenderSetupCallback(Windows::Foundation::EventHandler<MicrosoftDisplayCaptureTools::Display::IDisplayEnginePropertySet> const& handler);
        void RenderSetupCallback(event_token const& token) noexcept;

        event_token RenderLoopCallback(Windows::Foundation::EventHandler<MicrosoftDisplayCaptureTools::Display::IDisplayEnginePropertySet> const& handler);
        void RenderLoopCallback(event_token const& token) noexcept;

    private:
        void RefreshTarget();
        void ConnectTarget();
        void RefreshMode();
        void PrepareRender();
        void RenderLoop();

    private:
        const MicrosoftDisplayCaptureTools::Framework::ILogger m_logger{nullptr};

        Windows::Devices::Display::Core::DisplayDevice m_displayDevice{nullptr};
        Windows::Devices::Display::Core::DisplayManager m_displayManager{nullptr};
        Windows::Devices::Display::Core::DisplayTarget m_displayTarget{nullptr};
        Windows::Devices::Display::Core::DisplayState m_displayState{nullptr};
        Windows::Devices::Display::Core::DisplayPath m_displayPath{nullptr};

        std::unique_ptr<MonitorUtilities::MonitorControl> m_monitorControl;

        com_ptr<DisplayEnginePropertySet> m_propertySet;

        event<Windows::Foundation::EventHandler<MicrosoftDisplayCaptureTools::Display::IDisplayEnginePropertySet>> m_displaySetupCallback;
        event<Windows::Foundation::EventHandler<MicrosoftDisplayCaptureTools::Display::IDisplayEnginePropertySet>> m_renderSetupCallback;
        event<Windows::Foundation::EventHandler<MicrosoftDisplayCaptureTools::Display::IDisplayEnginePropertySet>> m_renderLoopCallback;

        std::thread m_renderThread;
        std::atomic_bool m_valid = true;
        std::atomic_bool m_presenting = false;
    };

    struct RenderWatchdog : implements<RenderWatchdog, Windows::Foundation::IClosable>
    {
    private:
        DisplayOutput* m_displayOutput{nullptr};

    public:
        RenderWatchdog(DisplayOutput* displayOutput) : m_displayOutput(displayOutput){};

        ~RenderWatchdog()
        {
            Close();
        };

        void Close()
        {
            if (m_displayOutput)
                m_displayOutput->StopRender();
        };
    };

    struct DisplayEngine : DisplayEngineT<DisplayEngine>
    {
        DisplayEngine();
        DisplayEngine(MicrosoftDisplayCaptureTools::Framework::ILogger const& logger);

        ~DisplayEngine();

        hstring Name()
        {
            return L"BasicDisplayControl";
        };
        hstring Version()
        {
            return L"0.1";
        };

        MicrosoftDisplayCaptureTools::Display::IDisplayOutput InitializeOutput(Windows::Devices::Display::Core::DisplayTarget const& target);
        MicrosoftDisplayCaptureTools::Display::IDisplayOutput InitializeOutput(hstring target);
        void SetConfigData(Windows::Data::Json::IJsonValue data);

    private:
        const MicrosoftDisplayCaptureTools::Framework::ILogger m_logger{nullptr};
        Windows::Devices::Display::Core::DisplayManager m_displayManager{nullptr};
    };

    struct DisplayEngineFactory : DisplayEngineFactoryT<DisplayEngineFactory>
    {
        DisplayEngineFactory() = default;

        MicrosoftDisplayCaptureTools::Display::IDisplayEngine CreateDisplayEngine(MicrosoftDisplayCaptureTools::Framework::ILogger const& logger);
    };
}

namespace winrt::BasicDisplayControl::factory_implementation
{
    struct DisplayEngine : DisplayEngineT<DisplayEngine, implementation::DisplayEngine>
    {
    };
    struct DisplayEngineFactory : DisplayEngineFactoryT<DisplayEngineFactory, implementation::DisplayEngineFactory>
    {
    };
}