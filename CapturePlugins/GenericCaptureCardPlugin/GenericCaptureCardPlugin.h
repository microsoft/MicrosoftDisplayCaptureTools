#pragma once
#include "Controller.g.h"
#include "ControllerFactory.g.h"

namespace winrt::GenericCaptureCardPlugin::implementation {
// The per-channel error tolerance for this particular capture card.
// (20 is drastically unacceptable for a final product - the generic capture card
// I have on my desk right now compresses the stream like crazy)
constexpr uint8_t ColorChannelTolerance = 20;

struct DisplayCapture : implements<DisplayCapture, winrt::MicrosoftDisplayCaptureTools::CaptureCard::IDisplayCapture>
{
    DisplayCapture(
        winrt::Windows::Media::Capture::CapturedFrame frame,
        winrt::Windows::Foundation::Collections::IMap<winrt::hstring, winrt::Windows::Foundation::IInspectable> extendedProps);

    bool CompareCaptureToPrediction(winrt::hstring name, winrt::MicrosoftDisplayCaptureTools::ConfigurationTools::IPredictionData prediction);
    winrt::MicrosoftDisplayCaptureTools::Framework::IFrameData GetFrameData();
    winrt::Windows::Foundation::Collections::IMapView<winrt::hstring, winrt::Windows::Foundation::IInspectable> ExtendedProperties();

private:
    winrt::MicrosoftDisplayCaptureTools::Framework::FrameData m_frameData{nullptr};
    winrt::Windows::Foundation::Collections::IMap<winrt::hstring, winrt::Windows::Foundation::IInspectable> m_extendedProps{nullptr};
};

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
    void SetDescriptor(winrt::MicrosoftDisplayCaptureTools::Framework::IMonitorDescriptor descriptor)
    {
        Logger().LogAssert(L"SetDescriptor not available for this capture card.");
    }

private:
    winrt::hstring m_deviceId;
    winrt::hstring m_deviceFriendlyName;

    winrt::com_ptr<CaptureCapabilities> m_captureCapabilities;
    winrt::com_ptr<CaptureTrigger> m_captureTrigger;
    winrt::com_ptr<DisplayCapture> m_capture;

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
