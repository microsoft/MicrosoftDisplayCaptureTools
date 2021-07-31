#pragma once
#include "ConfigurationToolbox.g.h"

namespace winrt::Toolbox::implementation
{
    struct ConfigurationToolbox : ConfigurationToolboxT<ConfigurationToolbox>
    {
        ConfigurationToolbox() = default;

        hstring Name();
        com_array<hstring> GetSupportedTools();
        MicrosoftDisplayCaptureTools::ConfigurationTools::IConfigurationTool GetTool(hstring const& toolName);
    };
}
namespace winrt::Toolbox::factory_implementation
{
    struct ConfigurationToolbox : ConfigurationToolboxT<ConfigurationToolbox, implementation::ConfigurationToolbox>
    {
    };
}
