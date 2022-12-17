#pragma once
#include "Controller.g.h"
#include "ControllerFactory.g.h"

namespace winrt::GenericCaptureCardPlugin::implementation {
// The per-channel error tolerance for this particular capture card.
// (20 is drastically unacceptable for a final product - the generic capture card
// I have on my desk right now compresses the stream like crazy)
constexpr uint8_t ColorChannelTolerance = 100;

struct DisplayCapture : implements<DisplayCapture, winrt::MicrosoftDisplayCaptureTools::CaptureCard::IDisplayCapture>
{
    DisplayCapture(
        winrt::Windows::Media::Capture::CapturedFrame frame,
        winrt::MicrosoftDisplayCaptureTools::Framework::ILogger const& logger,
        winrt::Windows::Foundation::Collections::IMap<winrt::hstring, winrt::Windows::Foundation::IInspectable> extendedProps);

    bool CompareCaptureToPrediction(winrt::hstring name, winrt::MicrosoftDisplayCaptureTools::Display::IDisplayEnginePrediction prediction);
    winrt::Windows::Foundation::IMemoryBufferReference GetRawPixelData();
    winrt::Windows::Foundation::Collections::IMapView<winrt::hstring, winrt::Windows::Foundation::IInspectable> ExtendedProperties();

private:
    winrt::Windows::Graphics::Imaging::SoftwareBitmap m_bitmap{nullptr};
    const winrt::MicrosoftDisplayCaptureTools::Framework::ILogger m_logger{nullptr};
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
    void SetDescriptor(winrt::MicrosoftDisplayCaptureTools::Framework::IMonitorDescriptor descriptor)
    {
        m_logger.LogAssert(L"SetDescriptor not available for this capture card.");
    }

private:
    winrt::hstring m_deviceId;
    winrt::hstring m_deviceFriendlyName;

    winrt::com_ptr<CaptureCapabilities> m_captureCapabilities;
    winrt::com_ptr<CaptureTrigger> m_captureTrigger;
    winrt::com_ptr<DisplayCapture> m_capture;

    winrt::Windows::Media::Capture::MediaCapture m_mediaCapture{nullptr};

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
    std::vector<winrt::MicrosoftDisplayCaptureTools::CaptureCard::IDisplayInput> m_displayInputs;

    winrt::hstring m_usbIdFromConfigFile = L"";

    const winrt::MicrosoftDisplayCaptureTools::Framework::ILogger m_logger{nullptr};
};

struct ControllerFactory : ControllerFactoryT<ControllerFactory>
{
    ControllerFactory() = default;

    winrt::MicrosoftDisplayCaptureTools::CaptureCard::IController CreateController(winrt::MicrosoftDisplayCaptureTools::Framework::ILogger const& logger);
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
