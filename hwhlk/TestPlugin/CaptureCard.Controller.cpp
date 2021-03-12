#include "pch.h"
#include "CaptureCard.Controller.h"
#include "CaptureCard.Controller.g.cpp"

namespace winrt::CaptureCard::implementation
{
    hstring Controller::Name()
    {
        throw hresult_not_implemented();
    }
    void Controller::Name(hstring const& value)
    {
        throw hresult_not_implemented();
    }
    void Controller::EnumerateDisplayInputs(array_view<CaptureCard::DisplayInput> displays)
    {
        throw hresult_not_implemented();
    }
    ConfigurationTools::ConfigurationToolbox Controller::GetToolbox()
    {
        throw hresult_not_implemented();
    }
}
