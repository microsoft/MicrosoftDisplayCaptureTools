#include "pch.h"
#include "Controller.g.cpp"

using namespace winrt::MicrosoftDisplayCaptureTools;
using namespace winrt::MicrosoftDisplayCaptureTools::CaptureCard;

namespace winrt::CaptureCard::implementation
{
    Controller::Controller()
    {
        //
        // Normally this is where a capture card would initialize and determine its own capabilities and 
        // inputs. For this sample, we are reporting a single input to this 'fake' capture card.
        //
        auto input = winrt::make<SampleDisplayInput>();

        m_displayInputs.push_back(input);
    }

    hstring Controller::Name()
    {
        return L"Software Test Plugin";
    }

    com_array<IDisplayInput> Controller::EnumerateDisplayInputs()
    {
        auto ret = winrt::com_array<IDisplayInput>(m_displayInputs);
        return ret;
    }

    ConfigurationTools::IConfigurationToolbox Controller::GetToolbox()
    {
        return nullptr;
    }
}
