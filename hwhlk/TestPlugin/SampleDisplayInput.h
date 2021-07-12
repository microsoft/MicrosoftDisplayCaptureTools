#pragma once

namespace winrt::CaptureCard::implementation
{
    struct SampleDisplayInput : implements<SampleDisplayInput, IDisplayInput>
    {
        SampleDisplayInput(std::weak_ptr<FrankenboardDevice> parent);

        hstring Name();

        Windows::Devices::Display::Core::DisplayTarget MapCaptureInputToDisplayPath();

        CaptureCapabilities GetCapabilities();

        IDisplayCapture CaptureFrame(CaptureTrigger trigger);

        void FinalizeDisplayState();

        std::weak_ptr<FrankenboardDevice> m_parent;
    };
}
