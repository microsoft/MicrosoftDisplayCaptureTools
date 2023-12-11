#pragma once
#include "Controller.g.h"
#include "ControllerFactory.g.h"

namespace winrt::GenericCaptureCardPlugin::implementation
{
    // The psnr limit we use to determine if a match is good enough to be considered a match.
    constexpr double PsnrLimit = 35.0;

    struct CaptureTrigger : implements<CaptureTrigger, winrt::MicrosoftDisplayCaptureTools::CaptureCard::ICaptureTrigger>
    {
        CaptureTrigger() = default;

        winrt::MicrosoftDisplayCaptureTools::CaptureCard::CaptureTriggerType Type();
        void Type(winrt::MicrosoftDisplayCaptureTools::CaptureCard::CaptureTriggerType type);

        uint32_t TimeToCapture();
        void TimeToCapture(uint32_t time);

    private:
        winrt::MicrosoftDisplayCaptureTools::CaptureCard::CaptureTriggerType m_type{
            winrt::MicrosoftDisplayCaptureTools::CaptureCard::CaptureTriggerType::Immediate};
        uint32_t m_time{0};
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
    
        void ValidateAgainstDisplayOutput(winrt::MicrosoftDisplayCaptureTools::Display::IDisplayOutput displayOutput){};
    };

    struct DisplayInput : implements<DisplayInput, winrt::MicrosoftDisplayCaptureTools::CaptureCard::IDisplayInput>
    {
        DisplayInput(winrt::hstring deviceId);

        winrt::hstring Name();

        void FinalizeDisplayState();

        winrt::MicrosoftDisplayCaptureTools::CaptureCard::ICaptureCapabilities GetCapabilities();
        winrt::MicrosoftDisplayCaptureTools::CaptureCard::ICaptureTrigger GetCaptureTrigger();
        winrt::MicrosoftDisplayCaptureTools::CaptureCard::IDisplayCapture CaptureFrame();

        // Methods that this particular capture card can't support
        void SetDescriptor(winrt::MicrosoftDisplayCaptureTools::Framework::IMonitorDescriptor descriptor);

    private:
        winrt::hstring m_deviceId;
        winrt::hstring m_deviceFriendlyName;

        winrt::Windows::Media::Capture::MediaCapture m_mediaCapture{nullptr};
    };

    struct Controller : ControllerT<Controller>
    {
        Controller();

        hstring Name();
        com_array<winrt::MicrosoftDisplayCaptureTools::CaptureCard::IDisplayInput> EnumerateDisplayInputs();
        void SetConfigData(winrt::Windows::Data::Json::IJsonValue data);

        MicrosoftDisplayCaptureTools::Framework::Version Version()
        {
            return MicrosoftDisplayCaptureTools::Framework::Version(0, 1, 0);
        };

    private:
        std::vector<winrt::MicrosoftDisplayCaptureTools::CaptureCard::IDisplayInput> m_displayInputs;

        winrt::hstring m_usbIdFromConfigFile = L"";
    };

    struct ControllerFactory : ControllerFactoryT<ControllerFactory>
    {
        ControllerFactory() = default;

        winrt::MicrosoftDisplayCaptureTools::CaptureCard::IController CreateController();
    };
    } // namespace winrt::GenericCaptureCardPlugin::implementation

    namespace winrt::GenericCaptureCardPlugin::factory_implementation {
    struct Controller : ControllerT<Controller, implementation::Controller>
    {
    };
    struct ControllerFactory : ControllerFactoryT<ControllerFactory, implementation::ControllerFactory>
    {
    };
} // namespace winrt::GenericCaptureCardPlugin::factory_implementation
