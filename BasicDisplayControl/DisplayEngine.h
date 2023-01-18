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

    struct DisplayEnginePlaneBaseImage : implements<DisplayEnginePlaneBaseImage, MicrosoftDisplayCaptureTools::Display::IDisplayEnginePlaneBaseImage>
    {
        DisplayEnginePlaneBaseImage() {}

        Windows::Storage::Streams::IBuffer Pixels() { return m_pixels; }
        void Pixels(Windows::Storage::Streams::IBuffer buffer) { m_pixels = buffer; }

        Windows::Graphics::SizeInt32 Resolution() { return m_resolution; }
        void Resolution(Windows::Graphics::SizeInt32 resolution) { m_resolution = resolution; }

        Windows::Graphics::DirectX::DirectXPixelFormat Format() { return m_format; }
        void Format(Windows::Graphics::DirectX::DirectXPixelFormat format) { m_format = format; }

    private:
        Windows::Storage::Streams::IBuffer m_pixels{nullptr};
        Windows::Graphics::SizeInt32 m_resolution{0, 0};
        Windows::Graphics::DirectX::DirectXPixelFormat m_format{Windows::Graphics::DirectX::DirectXPixelFormat::Unknown};
    };

    struct PredictedImagePixelData : implements <PredictedImagePixelData, 
        MicrosoftDisplayCaptureTools::Framework::IPixelData, MicrosoftDisplayCaptureTools::Framework::IPixelDataExtension>
    {
        PredictedImagePixelData(MicrosoftDisplayCaptureTools::Framework::ILogger const& logger) : m_logger(logger){}

        // --------------------------------------------Functions from IPixelData -----------------------------------------------------
        Windows::Storage::Streams::IBuffer Pixels() { return m_pixels; }
        void Pixels(Windows::Storage::Streams::IBuffer buffer) { m_pixels = buffer; }

        Windows::Graphics::SizeInt32 Resolution() { return m_resolution; }
        void Resolution(Windows::Graphics::SizeInt32 resolution) { m_resolution = resolution; }

        MicrosoftDisplayCaptureTools::Framework::PixelDataDescription FormatDescription() { return m_description; }
        void FormatDescription(MicrosoftDisplayCaptureTools::Framework::PixelDataDescription description) { m_description = description; }

        // --------------------------------------------Functions from IPixelDataExtension --------------------------------------------
        // Create a facsimile of this pixel data in a BGRA8 format, for ease of rendering/debugging.
        Windows::Graphics::Imaging::SoftwareBitmap GetRenderableApproximation();

        // Get a byte array representing a specific pixel. It is up to the caller to properly interpret this using the
        // FormatDescription member.
        com_array<byte> GetSpecificPixel(uint32_t x, uint32_t y);

        // Return a new IPixelDataExtension object containing the delta between this object and other supplied image.
        MicrosoftDisplayCaptureTools::Framework::IPixelDataExtension GetImageDelta(MicrosoftDisplayCaptureTools::Framework::IPixelData other);

    private:
        const MicrosoftDisplayCaptureTools::Framework::ILogger m_logger{nullptr};
        Windows::Storage::Streams::IBuffer m_pixels{nullptr};
        Windows::Graphics::SizeInt32 m_resolution{0, 0};
        MicrosoftDisplayCaptureTools::Framework::PixelDataDescription m_description{0};
    };

    struct DisplayEnginePlanePropertySet : implements<DisplayEnginePlanePropertySet, MicrosoftDisplayCaptureTools::Display::IDisplayEnginePlanePropertySet>
    {
        DisplayEnginePlanePropertySet(MicrosoftDisplayCaptureTools::Framework::ILogger const& logger) : 
            m_logger(logger), m_baseImage(make<DisplayEnginePlaneBaseImage>()){}

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

        // The base image is another required property, but is not trivially constructed like the others, instead the object will be created
        // when retrieved and the caller is expected to use the type members to modify it.
        MicrosoftDisplayCaptureTools::Display::IDisplayEnginePlaneBaseImage BaseImage();

        // TODO insert generic property bag.

    private:
        const MicrosoftDisplayCaptureTools::Framework::ILogger m_logger{nullptr};
        bool m_active = false;
        Windows::Graphics::Imaging::BitmapBounds m_rect{0};
        Windows::Graphics::DirectX::DirectXPixelFormat m_format{ Windows::Graphics::DirectX::DirectXPixelFormat::Unknown };
        MicrosoftDisplayCaptureTools::Display::IDisplayEnginePlaneBaseImage m_baseImage;
    };

    struct DisplayEnginePlaneCapabilities : implements<DisplayEnginePlaneCapabilities, MicrosoftDisplayCaptureTools::Display::IDisplayEnginePlaneCapabilities>
    {
        DisplayEnginePlaneCapabilities(MicrosoftDisplayCaptureTools::Framework::ILogger const& logger) :
            m_logger(logger){}

        ~DisplayEnginePlaneCapabilities()
        {
        }

        // TODO: this is a placeholder for the time being
        hstring Name()
        {
            return L"Placeholder";
        };

    private:
        const MicrosoftDisplayCaptureTools::Framework::ILogger m_logger{nullptr};
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

    struct DisplayEngineCapabilities : implements<DisplayEngineCapabilities, MicrosoftDisplayCaptureTools::Display::IDisplayEngineCapabilities>
    {
        DisplayEngineCapabilities(MicrosoftDisplayCaptureTools::Framework::ILogger const& logger);

        com_array<Windows::Devices::Display::Core::DisplayModeInfo> GetSupportedModes();
        com_array<MicrosoftDisplayCaptureTools::Display::IDisplayEnginePlaneCapabilities> GetPlaneCapabilities();

        std::vector<com_ptr<DisplayEnginePlaneCapabilities>> m_planeCapabilities;
        std::vector<Windows::Devices::Display::Core::DisplayModeInfo> m_modes;

    private:
        const MicrosoftDisplayCaptureTools::Framework::ILogger m_logger{nullptr};
    };

    struct DisplayEnginePrediction : implements<DisplayEnginePrediction, MicrosoftDisplayCaptureTools::Display::IDisplayEnginePrediction>
    {
        DisplayEnginePrediction(DisplayEnginePropertySet* properties, MicrosoftDisplayCaptureTools::Framework::ILogger const& logger);

        Windows::Graphics::Imaging::SoftwareBitmap GetBitmap();

    private:
        Windows::Graphics::Imaging::SoftwareBitmap m_bitmap{ nullptr };

        const MicrosoftDisplayCaptureTools::Framework::ILogger m_logger{nullptr};
    };

    //
    // This renderer class handles the connection to a physical display through the GPU. This class represents a render loop
    // drawing to the target output. Note that this is an implementation detail - other DisplayEngine implementations are
    // welcome to use any means of getting pixels on screen - this uses D3D11 encapsulated in this 'Renderer'.
    //
    struct Renderer : implements<Renderer, Windows::Foundation::IClosable>
    {
    public: // Constructors/Destructors/IClosable
        Renderer(MicrosoftDisplayCaptureTools::Framework::ILogger const& logger) : m_logger(logger){};

        ~Renderer()
        {
            Close();
        }
        void Close();

    public: // Utility functions
        void RefreshMode();
        void StartRender(DisplayEnginePropertySet* properties);

    public: // Direct Display Objects
        Windows::Devices::Display::Core::DisplayManager displayManager{ nullptr };
        Windows::Devices::Display::Core::DisplayTarget displayTarget{ nullptr };
        Windows::Devices::Display::Core::DisplayState displayState{ nullptr };
        Windows::Devices::Display::Core::DisplayPath displayPath{ nullptr };
        Windows::Devices::Display::Core::DisplayDevice displayDevice{ nullptr };
        Windows::Devices::Display::Core::DisplayModeInfo displayMode{ nullptr };

    public: // D3D11 Objects
        com_ptr<ID3D11Device5> m_d3dDevice;
        com_ptr<ID3D11DeviceContext> m_d3dContext;
        com_ptr<ID3D11Texture2D> m_d3dSurface;
        com_ptr<ID3D11RenderTargetView> m_d3dRenderTarget;
        com_ptr<ID3D11Fence> m_d3dFence;
        uint64_t m_d3dFenceValue = 0;

    private: // Tool-set properties for this test rendering
        DisplayEnginePropertySet* m_properties{ nullptr };

    private: // Render thread handlers
        void Render();
        std::thread renderThread;
        std::atomic_bool m_valid = true;
        std::atomic_bool m_presenting = false;

        const MicrosoftDisplayCaptureTools::Framework::ILogger m_logger{nullptr};
    };

    struct DisplayOutput : implements<DisplayOutput, MicrosoftDisplayCaptureTools::Display::IDisplayOutput>
    {
        DisplayOutput(
            MicrosoftDisplayCaptureTools::Framework::ILogger const& logger, 
            Windows::Devices::Display::Core::DisplayTarget const& target,
            Windows::Devices::Display::Core::DisplayManager const& manager);

        ~DisplayOutput();

        Windows::Devices::Display::Core::DisplayTarget Target();
        MicrosoftDisplayCaptureTools::Display::IDisplayEngineCapabilities GetCapabilities();
        MicrosoftDisplayCaptureTools::Display::IDisplayEnginePropertySet GetProperties();
        MicrosoftDisplayCaptureTools::Display::IDisplayEnginePrediction GetPrediction();

        Windows::Foundation::IClosable StartRender();

    private:
        void RefreshTarget();
        void ConnectTarget();
        void PopulateCapabilities();

    private:
        const MicrosoftDisplayCaptureTools::Framework::ILogger m_logger{nullptr};

        Windows::Devices::Display::Core::DisplayDevice m_displayDevice{nullptr};
        Windows::Devices::Display::Core::DisplayManager m_displayManager{nullptr};
        Windows::Devices::Display::Core::DisplayTarget m_displayTarget{nullptr};
        Windows::Devices::Display::Core::DisplayState m_displayState{nullptr};
        Windows::Devices::Display::Core::DisplayPath m_displayPath{nullptr};

        std::unique_ptr<MonitorUtilities::MonitorControl> m_monitorControl;

        com_ptr<DisplayEngineCapabilities> m_capabilities;
        com_ptr<DisplayEnginePropertySet> m_propertySet;
    };

    struct DisplayEngine : DisplayEngineT<DisplayEngine>
    {
        DisplayEngine();
        DisplayEngine(MicrosoftDisplayCaptureTools::Framework::ILogger const& logger);

        ~DisplayEngine();

        hstring Name()
        {
            return L"BasicDisplayControl";
        }
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