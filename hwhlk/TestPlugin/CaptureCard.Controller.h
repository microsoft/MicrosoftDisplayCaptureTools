#pragma once
#include "Controller.g.h"

namespace winrt::CaptureCard::implementation
{
    struct Controller : ControllerT<Controller>
    {
        Controller();

        hstring Name();
        com_array<MicrosoftDisplayCaptureTools::CaptureCard::IDisplayInput> EnumerateDisplayInputs();
        MicrosoftDisplayCaptureTools::ConfigurationTools::IConfigurationToolbox GetToolbox();

        std::vector<MicrosoftDisplayCaptureTools::CaptureCard::IDisplayInput> m_displayInputs;
    };
}
namespace winrt::CaptureCard::factory_implementation
{
    struct Controller : ControllerT<Controller, implementation::Controller>
    {
    };
}
