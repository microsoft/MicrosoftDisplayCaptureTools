#include "pch.h"
#include "SampleDisplayInput.h"
#include "SampleDisplayCapture.h"

using namespace winrt::MicrosoftDisplayCaptureTools;
using namespace winrt::MicrosoftDisplayCaptureTools::CaptureCard;

namespace winrt::TestPlugin::implementation
{
    SampleDisplayInput::SampleDisplayInput(std::weak_ptr<FrankenboardDevice> parent)
        : m_parent(parent)
    {
    }

    hstring SampleDisplayInput::Name()
    {
        return L"HDMI";
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
            m_parent.lock()->TriggerHdmiCapture();
            return winrt::make<SampleDisplayCapture>(m_parent.lock());

        case CaptureTriggerType::Timer:
            SleepEx(trigger.timeToCapture, FALSE);
            m_parent.lock()->TriggerHdmiCapture();
            return winrt::make<SampleDisplayCapture>(m_parent.lock());
        }

        throw winrt::hresult_not_implemented();
    }

    void SampleDisplayInput::FinalizeDisplayState()
    {
        // This where a real capture card would ensure that the current display characteristics match
        // any characteristics set by tools so far. For example, if a resolution-setting tool were to 
        // be exposed, this function should ensure that the display presented to the OS _does_ support
        // that resolution.
    }
}