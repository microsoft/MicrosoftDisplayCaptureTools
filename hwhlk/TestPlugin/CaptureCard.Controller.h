#pragma once
#include "CaptureCard.Controller.g.h"

namespace winrt::CaptureCard::implementation
{
    struct Controller : ControllerT<Controller>
    {
        Controller();

        hstring Name();
        com_array<CaptureCard::DisplayInput> EnumerateDisplayInputs();
        ConfigurationTools::ConfigurationToolbox GetToolbox();

        std::vector<CaptureCard::DisplayInput> m_displayInputs;
    };
}
namespace winrt::CaptureCard::factory_implementation
{
    struct Controller : ControllerT<Controller, implementation::Controller>
    {
    };
}
