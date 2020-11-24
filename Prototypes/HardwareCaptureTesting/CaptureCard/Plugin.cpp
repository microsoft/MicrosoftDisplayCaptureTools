#include "pch.h"
#include "Plugin.h"
#include "Plugin.g.cpp"

namespace winrt::HardwareCaptureTesting::CaptureCard::implementation
{
    com_array<uint32_t> Plugin::DiscoverDisplays()
    {
        throw hresult_not_implemented();
    }
    void Plugin::CaptureCurrentState()
    {
        throw hresult_not_implemented();
    }
    Windows::Data::Json::JsonObject Plugin::GetCurrentState(uint32_t displayId)
    {
        throw hresult_not_implemented();
    }
    Windows::Data::Json::JsonObject Plugin::RunComparison(uint32_t displayId, Windows::Data::Json::JsonObject const& target)
    {
        throw hresult_not_implemented();
    }
}
