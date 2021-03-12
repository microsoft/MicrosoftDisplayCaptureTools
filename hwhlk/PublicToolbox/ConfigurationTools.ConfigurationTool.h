#pragma once
#include "ConfigurationTools.ConfigurationTool.g.h"

namespace winrt::ConfigurationTools::implementation
{
    struct ConfigurationTool : ConfigurationToolT<ConfigurationTool>
    {
        ConfigurationTool() = default;

        hstring Name();
        void Name(hstring const& value);
        ConfigurationTools::ConfigurationToolCategory Category();
        void Category(ConfigurationTools::ConfigurationToolCategory const& value);
        ConfigurationTools::ConfigurationToolRequirements Requirements();
        void Requirements(ConfigurationTools::ConfigurationToolRequirements const& value);
        void GetSupportedConfigurations(array_view<hstring> supportedConfigurations);
        void SetConfiguration(hstring const& configuration);
        void ApplyToHardware(Windows::Devices::Display::Core::DisplayTarget const& target);
        void ApplyToSoftwareReference(DisplayStateReference::StaticReference const& reference);
    };
}
