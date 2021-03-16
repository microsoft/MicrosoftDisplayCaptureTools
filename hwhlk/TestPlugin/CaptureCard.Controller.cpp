#include "pch.h"
#include "CaptureCard.Controller.g.cpp"

namespace winrt::CaptureCard::implementation
{
    Controller::Controller()
    {
        // This sample plugin only supports a single "Display".
        auto input = winrt::make<CaptureCard::implementation::DisplayInput>();
        auto customInterface = input.as<CaptureCard::ITestPluginInput>();
        customInterface.InitializeWithState(L"Display 1");

        m_displayInputs.push_back(input);
    }

    hstring Controller::Name()
    {
        return L"Software Test Plugin";
    }
    com_array<CaptureCard::DisplayInput> Controller::EnumerateDisplayInputs()
    {
        auto ret = winrt::com_array<CaptureCard::DisplayInput>(m_displayInputs);
        return ret;
    }
    ConfigurationTools::ConfigurationToolbox Controller::GetToolbox()
    {
        throw hresult_not_implemented();
    }
}
