#pragma once
#include "Plugin.g.h"

namespace winrt::HardwareCaptureTesting::CaptureCard::implementation
{
    struct Plugin : PluginT<Plugin>
    {
        Plugin() = default;

        com_array<uint32_t> DiscoverDisplays();
        void CaptureCurrentState();
        Windows::Data::Json::JsonObject GetCurrentState(uint32_t displayId);
        Windows::Data::Json::JsonObject RunComparison(uint32_t displayId, Windows::Data::Json::JsonObject const& target);
    };
}
namespace winrt::HardwareCaptureTesting::CaptureCard::factory_implementation
{
    struct Plugin : PluginT<Plugin, implementation::Plugin>
    {
    };
}
