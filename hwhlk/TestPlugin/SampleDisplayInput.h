#pragma once

namespace winrt::CaptureCard::implementation
{
    struct SampleDisplayInput : implements<SampleDisplayInput, MicrosoftDisplayCaptureTools::CaptureCard::IDisplayInput>
    {
        SampleDisplayInput() = default;

        hstring Name();

        Windows::Devices::Display::Core::DisplayTarget MapCaptureInputToDisplayPath();

        MicrosoftDisplayCaptureTools::CaptureCard::CaptureCapabilities GetCapabilities();

        MicrosoftDisplayCaptureTools::CaptureCard::IDisplayCapture CaptureFrame(MicrosoftDisplayCaptureTools::CaptureCard::CaptureTrigger trigger);

        void FinalizeDisplayState();
    };
}
