#include "pch.h"
#include "CaptureCard.Controller.g.cpp"

namespace winrt::CaptureCard::implementation
{
    Controller::Controller()
    {
        //
        // Normally this is where a capture card would initialize and determine its own capabilities and 
        // inputs. For this sample, we are reporting a single input to this 'fake' capture card.
        //
        auto input = winrt::make<CaptureCard::implementation::SampleDisplayInput>();

        m_displayInputs.push_back(input);
    }

    hstring Controller::Name()
    {
        return L"Software Test Plugin";
    }
    com_array<CaptureCard::IDisplayInput> Controller::EnumerateDisplayInputs()
    {
        auto ret = winrt::com_array<CaptureCard::IDisplayInput>(m_displayInputs);
        return ret;
    }
    ConfigurationTools::ConfigurationToolbox Controller::GetToolbox()
    {
        throw hresult_not_implemented();
    }
}
