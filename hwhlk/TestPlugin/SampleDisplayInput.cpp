#include "pch.h"
#include "SampleDisplayInput.h"

namespace winrt::CaptureCard::implementation
{
    hstring SampleDisplayInput::Name()
    {
        return L"Sample Input";
    }

    Windows::Devices::Display::Core::DisplayTarget SampleDisplayInput::MapCaptureInputToDisplayPath()
    {
        throw winrt::hresult_not_implemented();
    }

    CaptureCapabilities SampleDisplayInput::GetCapabilities()
    {
        CaptureCapabilities caps;
        caps.captureFrameSeries = false;
        caps.returnFramesToHost = true;
        caps.returnRawFramesToHost = true;
        
        return caps;
    }

    IDisplayCapture SampleDisplayInput::CaptureFrame(CaptureTrigger trigger)
    {
        //
        // This is where a real capture card would capture an output frame, once the trigger condition
        // were met.
        //

        switch (trigger.type)
        {
        case CaptureTriggerType::FirstNonEmpty:
        case CaptureTriggerType::Immediate:
            return winrt::make<SampleDisplayCapture>();

        case CaptureTriggerType::Timer:
            SleepEx(trigger.timeToCapture, FALSE);
            return winrt::make<SampleDisplayCapture>();
        }

        throw winrt::hresult_not_implemented();
    }
}