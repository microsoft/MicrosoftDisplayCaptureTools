#pragma once
#include "CaptureCard.Controller.g.h"

namespace winrt::CaptureCard::implementation
{
    struct Controller : ControllerT<Controller>
    {
        Controller();

        hstring Name();
        com_array<CaptureCard::IDisplayInput> EnumerateDisplayInputs();
        ConfigurationTools::ConfigurationToolbox GetToolbox();

        std::vector<CaptureCard::IDisplayInput> m_displayInputs;
    };
}
namespace winrt::CaptureCard::factory_implementation
{
    struct Controller : ControllerT<Controller, implementation::Controller>
    {
    };
}
