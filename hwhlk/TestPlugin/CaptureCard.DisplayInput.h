#pragma once
#include "CaptureCard.DisplayInput.g.h"

namespace winrt::CaptureCard::implementation
{
    struct DisplayInput : DisplayInputT<DisplayInput>
    {
        DisplayInput() = default;

        hstring Name();
        void Name(hstring const& value);
        Windows::Devices::Display::Core::DisplayTarget MapCaptureInputToDisplayPath();
        CaptureCard::CaptureCapabilities GetCapabilities();
        CaptureCard::DisplayCapture CaptureFrame(CaptureCard::CaptureTrigger const& trigger);
    };
}
