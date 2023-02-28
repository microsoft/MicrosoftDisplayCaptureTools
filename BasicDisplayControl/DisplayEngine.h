#pragma once
#include "DisplayEngine.g.h"
#include "DisplayEngineFactory.g.h"

#include "..\Shared\Inc\DisplayEngineInterop.h"

namespace MonitorUtilities
{
    class MonitorControl;
}

namespace winrt::BasicDisplayControl::implementation
{
    struct DisplayPredictionData : implements<DisplayPredictionData, MicrosoftDisplayCaptureTools::Display::IDisplayPredictionData>
    {
        DisplayPredictionData(MicrosoftDisplayCaptureTools::Framework::ILogger const& logger);

        MicrosoftDisplayCaptureTools::Framework::IFrameData FrameData();

        Windows::Foundation::Collections::IMap<hstring, IInspectable> Properties();

    private:
        const MicrosoftDisplayCaptureTools::Framework::ILogger m_logger{nullptr};

        MicrosoftDisplayCaptureTools::Framework::IFrameData m_frameData{nullptr};
        Windows::Foundation::Collections::IMap<hstring, IInspectable> m_properties;
    };

    struct DisplayPrediction : implements<DisplayPrediction, MicrosoftDisplayCaptureTools::Display::IDisplayPrediction>
    {
        DisplayPrediction(MicrosoftDisplayCaptureTools::Framework::ILogger const& logger);

        Windows::Foundation::IAsyncOperation<MicrosoftDisplayCaptureTools::Display::IDisplayPredictionData> GeneratePredictionDataAsync();

        event_token DisplaySetupCallback(Windows::Foundation::EventHandler<MicrosoftDisplayCaptureTools::Display::IDisplayPredictionData> const& handler);
        void DisplaySetupCallback(event_token const& token) noexcept;

        event_token RenderSetupCallback(Windows::Foundation::EventHandler<MicrosoftDisplayCaptureTools::Display::IDisplayPredictionData> const& handler);
        void RenderSetupCallback(event_token const& token) noexcept;

        event_token RenderLoopCallback(Windows::Foundation::EventHandler<MicrosoftDisplayCaptureTools::Display::IDisplayPredictionData> const& handler);
        void RenderLoopCallback(event_token const& token) noexcept;

    private:
        const MicrosoftDisplayCaptureTools::Framework::ILogger m_logger{nullptr};

        event<Windows::Foundation::EventHandler<MicrosoftDisplayCaptureTools::Display::IDisplayPredictionData>> m_displaySetupCallback;
        event<Windows::Foundation::EventHandler<MicrosoftDisplayCaptureTools::Display::IDisplayPredictionData>> m_renderSetupCallback;
        event<Windows::Foundation::EventHandler<MicrosoftDisplayCaptureTools::Display::IDisplayPredictionData>> m_renderLoopCallback;
    };

    struct DisplayEnginePlaneProperties
        : implements<DisplayEnginePlaneProperties, MicrosoftDisplayCaptureTools::Display::IDisplayEnginePlaneProperties, MicrosoftDisplayCaptureTools::Display::IDisplayEngineInterop>
    {
        DisplayEnginePlaneProperties(MicrosoftDisplayCaptureTools::Framework::ILogger const& logger);

        ~DisplayEnginePlaneProperties()
        {
        }

        // Required plane properties
        bool Active();
        void Active(bool active);        
        Windows::Graphics::Imaging::BitmapBounds Rect();
        void Rect(Windows::Graphics::Imaging::BitmapBounds bounds);
        Windows::Foundation::Collections::IMap<hstring, IInspectable> Properties();

        // Properties defined in the interop header
        HRESULT __stdcall GetPlaneTexture(ID3D11Texture2D** texture) noexcept override;

        // Internal-only functions
        void SetPlaneTexture(ID3D11Texture2D* texture);

    private:
        const MicrosoftDisplayCaptureTools::Framework::ILogger m_logger{nullptr};
        bool m_active = false;
        Windows::Graphics::Imaging::BitmapBounds m_rect{0};
        Windows::Graphics::DirectX::DirectXPixelFormat m_format{ Windows::Graphics::DirectX::DirectXPixelFormat::Unknown };
        Windows::Foundation::Collections::IMap<hstring, IInspectable> m_Properties;

        winrt::com_ptr<ID3D11Texture2D> m_planeTexture;
    };

    struct DisplaySetupToolArgs : implements<DisplaySetupToolArgs, MicrosoftDisplayCaptureTools::Display::IDisplaySetupToolArgs>
    {
        DisplaySetupToolArgs(MicrosoftDisplayCaptureTools::Framework::ILogger const& logger,
            MicrosoftDisplayCaptureTools::Display::IDisplayEngineProperties properties,
            Windows::Devices::Display::Core::DisplayModeInfo const& mode) :
            m_logger(logger),
            m_properties(properties),
            m_mode(mode)
        {};

        MicrosoftDisplayCaptureTools::Display::IDisplayEngineProperties Properties()
        {
            return m_properties;
        }

        Windows::Devices::Display::Core::DisplayModeInfo Mode()
        {
            return m_mode;
        }

        void IsModeCompatible(bool accept);

        bool Compatible()
        {
            return m_compatibility;
        }

    private:
        const MicrosoftDisplayCaptureTools::Framework::ILogger m_logger{nullptr};
        MicrosoftDisplayCaptureTools::Display::IDisplayEngineProperties m_properties;
        const Windows::Devices::Display::Core::DisplayModeInfo m_mode;

        std::atomic_bool m_compatibility = true;
    };

    struct RenderSetupToolArgs : implements<RenderSetupToolArgs, MicrosoftDisplayCaptureTools::Display::IRenderSetupToolArgs>
    {
        RenderSetupToolArgs(MicrosoftDisplayCaptureTools::Framework::ILogger const& logger, MicrosoftDisplayCaptureTools::Display::IDisplayEngineProperties properties) :
            m_logger(logger), m_properties(properties){};

        MicrosoftDisplayCaptureTools::Display::IDisplayEngineProperties Properties()
        {
            return m_properties;
        }

    private:
        const MicrosoftDisplayCaptureTools::Framework::ILogger m_logger{nullptr};
        MicrosoftDisplayCaptureTools::Display::IDisplayEngineProperties m_properties;
    };

    struct RenderingToolArgs : implements<RenderingToolArgs, MicrosoftDisplayCaptureTools::Display::IRenderingToolArgs>
    {
        RenderingToolArgs(MicrosoftDisplayCaptureTools::Framework::ILogger const& logger, MicrosoftDisplayCaptureTools::Display::IDisplayEngineProperties properties) :
            m_logger(logger), m_properties(properties){};

        MicrosoftDisplayCaptureTools::Display::IDisplayEngineProperties Properties()
        {
            return m_properties;
        }

        uint64_t FrameNumber()
        {
            return m_frameNumber;
        };

        void FrameNumber(uint64_t frameNumber)
        {
            m_frameNumber = frameNumber;
        };

    private:
        uint64_t m_frameNumber = 0;
        const MicrosoftDisplayCaptureTools::Framework::ILogger m_logger{nullptr};
        MicrosoftDisplayCaptureTools::Display::IDisplayEngineProperties m_properties;
    };

    struct DisplayEngineProperties : implements<DisplayEngineProperties, MicrosoftDisplayCaptureTools::Display::IDisplayEngineProperties>
    {
        DisplayEngineProperties(MicrosoftDisplayCaptureTools::Framework::ILogger const& logger);

        Windows::Devices::Display::Core::DisplayModeInfo ActiveMode();
        void ActiveMode(Windows::Devices::Display::Core::DisplayModeInfo mode);

        double RefreshRate();
        void RefreshRate(double rate);

        Windows::Graphics::SizeInt32 Resolution();
        void Resolution(Windows::Graphics::SizeInt32 resolution);

        com_array<MicrosoftDisplayCaptureTools::Display::IDisplayEnginePlaneProperties> GetPlaneProperties();

        double m_refreshRate;
        Windows::Graphics::SizeInt32 m_resolution;
        Windows::Devices::Display::Core::DisplayModeInfo m_mode;
        std::vector<com_ptr<DisplayEnginePlaneProperties>> m_planeProperties;

        bool m_requeryMode;
        bool RequeryMode();

    private:
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
        MicrosoftDisplayCaptureTools::Display::IDisplayEngineProperties GetProperties();

        Windows::Foundation::IClosable StartRender();
        void StopRender();

        event_token DisplaySetupCallback(Windows::Foundation::EventHandler<MicrosoftDisplayCaptureTools::Display::IDisplaySetupToolArgs> const& handler);
        void DisplaySetupCallback(event_token const& token) noexcept;

        event_token RenderSetupCallback(Windows::Foundation::EventHandler<MicrosoftDisplayCaptureTools::Display::IRenderSetupToolArgs> const& handler);
        void RenderSetupCallback(event_token const& token) noexcept;

        event_token RenderLoopCallback(Windows::Foundation::EventHandler<MicrosoftDisplayCaptureTools::Display::IRenderingToolArgs> const& handler);
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

        com_ptr<DisplayEngineProperties> m_properties;

        event<Windows::Foundation::EventHandler<MicrosoftDisplayCaptureTools::Display::IDisplaySetupToolArgs>> m_displaySetupCallback;
        event<Windows::Foundation::EventHandler<MicrosoftDisplayCaptureTools::Display::IRenderSetupToolArgs>> m_renderSetupCallback;
        event<Windows::Foundation::EventHandler<MicrosoftDisplayCaptureTools::Display::IRenderingToolArgs>> m_renderLoopCallback;

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
        MicrosoftDisplayCaptureTools::Framework::Version Version()
        {
            return MicrosoftDisplayCaptureTools::Framework::Version(0, 1, 0);
        };

        MicrosoftDisplayCaptureTools::Display::IDisplayOutput InitializeOutput(Windows::Devices::Display::Core::DisplayTarget const& target);
        MicrosoftDisplayCaptureTools::Display::IDisplayOutput InitializeOutput(hstring target);

        MicrosoftDisplayCaptureTools::Display::IDisplayPrediction CreateDisplayPrediction();

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