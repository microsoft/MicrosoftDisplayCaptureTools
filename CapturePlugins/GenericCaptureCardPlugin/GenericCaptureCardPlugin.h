#pragma once
#include "Controller.g.h"
#include "ControllerFactory.g.h"

namespace winrt::GenericCaptureCardPlugin::implementation
{
    // The per-channel error tolerance for this particular capture card.
    // (20 is drastically unacceptable for a final product - the generic capture card
    // I have on my desk right now compresses the stream like crazy)
    constexpr uint8_t ColorChannelTolerance = 20;

    struct DisplayCapture : implements<DisplayCapture, winrt::MicrosoftDisplayCaptureTools::CaptureCard::IDisplayCapture>
    {
        DisplayCapture(winrt::Windows::Media::Capture::CapturedFrame frame, winrt::MicrosoftDisplayCaptureTools::Framework::ILogger const& logger);

        void CompareCaptureToPrediction(winrt::hstring name, winrt::MicrosoftDisplayCaptureTools::Display::IDisplayEnginePrediction prediction);
        winrt::Windows::Foundation::IMemoryBufferReference GetRawPixelData();

    private:
        winrt::Windows::Graphics::Imaging::SoftwareBitmap m_bitmap{nullptr};
        const winrt::MicrosoftDisplayCaptureTools::Framework::ILogger m_logger{nullptr};
    };

    struct CaptureTrigger : implements<CaptureTrigger, winrt::MicrosoftDisplayCaptureTools::CaptureCard::ICaptureTrigger>
    {
        CaptureTrigger() = default;

        winrt::MicrosoftDisplayCaptureTools::CaptureCard::CaptureTriggerType Type();
        void Type(winrt::MicrosoftDisplayCaptureTools::CaptureCard::CaptureTriggerType type);

        uint32_t TimeToCapture();
        void TimeToCapture(uint32_t time);

    private:
        winrt::MicrosoftDisplayCaptureTools::CaptureCard::CaptureTriggerType m_type{ winrt::MicrosoftDisplayCaptureTools::CaptureCard::CaptureTriggerType::Immediate };
        uint32_t m_time{ 0 };
    };

    struct CaptureCapabilities : implements<CaptureCapabilities, winrt::MicrosoftDisplayCaptureTools::CaptureCard::ICaptureCapabilities>
    {
        CaptureCapabilities() = default;

        bool CanReturnRawFramesToHost();
        bool CanReturnFramesToHost();
        bool CanCaptureFrameSeries();
        bool CanHotPlug();
        bool CanConfigureEDID();
        bool CanConfigureDisplayID();
        uint32_t GetMaxDescriptorSize();
    };

    struct DisplayInput : implements<DisplayInput, winrt::MicrosoftDisplayCaptureTools::CaptureCard::IDisplayInput>
    {
        DisplayInput(winrt::hstring deviceId, winrt::MicrosoftDisplayCaptureTools::Framework::ILogger const& logger);

        winrt::hstring Name();

        void FinalizeDisplayState();

        winrt::MicrosoftDisplayCaptureTools::CaptureCard::ICaptureCapabilities GetCapabilities();
        winrt::MicrosoftDisplayCaptureTools::CaptureCard::ICaptureTrigger GetCaptureTrigger();
        winrt::MicrosoftDisplayCaptureTools::CaptureCard::IDisplayCapture CaptureFrame();

        // Methods that this particular capture card can't support
        void SetDescriptor(winrt::MicrosoftDisplayCaptureTools::Framework::IMonitorDescriptor descriptor) { throw winrt::hresult_not_implemented(); }

    private:
        winrt::hstring m_deviceId;
        winrt::Windows::Media::Capture::MediaCapture m_mediaCapture{ nullptr };

        winrt::com_ptr<CaptureCapabilities> m_captureCapabilities;
        winrt::com_ptr<CaptureTrigger> m_captureTrigger;

        // This basic plugin can only handle 1 capture at a time.
        winrt::com_ptr<DisplayCapture> m_capture;

        const winrt::MicrosoftDisplayCaptureTools::Framework::ILogger m_logger{nullptr};
    };

    struct Controller : ControllerT<Controller>
    {
        Controller(); 
        Controller(winrt::MicrosoftDisplayCaptureTools::Framework::ILogger const& logger);

        hstring Name();
        com_array<winrt::MicrosoftDisplayCaptureTools::CaptureCard::IDisplayInput> EnumerateDisplayInputs();
        void SetConfigData(winrt::Windows::Data::Json::IJsonValue data);

        hstring Version()
        {
            return L"0.1";
        };  

    private:
        // Identifier that allows this plugin to identify the actual USB device. Here it is read from the supplied config file.
        winrt::hstring m_deviceId;

        // This plugin only supports a single display input.
        winrt::MicrosoftDisplayCaptureTools::CaptureCard::IDisplayInput m_displayInput;

        const winrt::MicrosoftDisplayCaptureTools::Framework::ILogger m_logger{nullptr};
    };

    struct ControllerFactory : ControllerFactoryT<ControllerFactory>
    {
        ControllerFactory() = default;

        winrt::MicrosoftDisplayCaptureTools::CaptureCard::IController CreateController(winrt::MicrosoftDisplayCaptureTools::Framework::ILogger const& logger);
    };
}

namespace winrt::GenericCaptureCardPlugin::factory_implementation
{
    struct Controller : ControllerT<Controller, implementation::Controller>
    {
    };
    struct ControllerFactory : ControllerFactoryT<ControllerFactory, implementation::ControllerFactory>
    {
    };
}
