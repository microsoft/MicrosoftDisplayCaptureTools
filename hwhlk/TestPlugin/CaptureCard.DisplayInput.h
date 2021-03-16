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

        //
        // Added to demonstrate adding custom communication between capture components - this can
        // be used and more methods added without interfering with the Framework/Plugin.
        //
        void InitializeWithState(hstring const& stuff);

        hstring m_name = L"";
    };
}
