#pragma once

namespace winrt::TestPlugin::implementation
{
    struct SampleDisplayInput : implements<SampleDisplayInput, MicrosoftDisplayCaptureTools::CaptureCard::IDisplayInput>
    {
        SampleDisplayInput(std::weak_ptr<FrankenboardDevice> parent);

        hstring Name();

        Windows::Devices::Display::Core::DisplayTarget MapCaptureInputToDisplayPath();

        MicrosoftDisplayCaptureTools::CaptureCard::CaptureCapabilities GetCapabilities();

        MicrosoftDisplayCaptureTools::CaptureCard::IDisplayCapture CaptureFrame(MicrosoftDisplayCaptureTools::CaptureCard::CaptureTrigger trigger);

        void FinalizeDisplayState();

        std::weak_ptr<FrankenboardDevice> m_parent;
    };
}
