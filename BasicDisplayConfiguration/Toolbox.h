#pragma once
#include "Toolbox.g.h"

namespace winrt::BasicDisplayConfiguration::implementation
{
    struct Toolbox : ToolboxT<Toolbox>
    {
        Toolbox() = default;

        hstring Name();
        com_array<hstring> GetSupportedTools();
        winrt::MicrosoftDisplayCaptureTools::ConfigurationTools::IConfigurationTool GetTool(hstring const& toolName);
        void SetConfigData(winrt::Windows::Data::Json::IJsonValue data);
    };
}
namespace winrt::BasicDisplayConfiguration::factory_implementation
{
    struct Toolbox : ToolboxT<Toolbox, implementation::Toolbox>
    {
    };
}
