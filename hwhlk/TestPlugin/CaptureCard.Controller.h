#pragma once
#include "CaptureCard.Controller.g.h"

namespace winrt::CaptureCard::implementation
{
    struct Controller : ControllerT<Controller>
    {
        Controller() = default;

        hstring Name();
        void Name(hstring const& value);
        void EnumerateDisplayInputs(array_view<CaptureCard::DisplayInput> displays);
        ConfigurationTools::ConfigurationToolbox GetToolbox();
    };
}
namespace winrt::CaptureCard::factory_implementation
{
    struct Controller : ControllerT<Controller, implementation::Controller>
    {
    };
}
