#pragma once

namespace winrt::CaptureCard::implementation
{
    struct SampleDisplayInput : implements<SampleDisplayInput, IDisplayInput>
    {
        SampleDisplayInput() = default;

        hstring Name();

        Windows::Devices::Display::Core::DisplayTarget MapCaptureInputToDisplayPath();

        CaptureCapabilities GetCapabilities();

        IDisplayCapture CaptureFrame(CaptureTrigger trigger);

        void FinalizeDisplayState();
    };
}
