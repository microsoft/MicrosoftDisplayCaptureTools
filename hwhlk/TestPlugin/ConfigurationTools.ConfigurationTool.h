#pragma once
#include "ConfigurationTools.ConfigurationTool.g.h"

namespace winrt::ConfigurationTools::implementation
{
    struct ConfigurationTool : ConfigurationToolT<ConfigurationTool>
    {
        ConfigurationTool() = default;

        hstring Name();
        ConfigurationTools::ConfigurationToolCategory Category();
        void Category(ConfigurationTools::ConfigurationToolCategory const& value);
        ConfigurationTools::ConfigurationToolRequirements Requirements();
        void Requirements(ConfigurationTools::ConfigurationToolRequirements const& value);
        com_array<hstring> GetSupportedConfigurations();
        void SetConfiguration(hstring const& configuration);
        void ApplyToHardware(Windows::Devices::Display::Core::DisplayTarget const& target);
        void ApplyToSoftwareReference(DisplayStateReference::StaticReference const& reference);
    };
}
