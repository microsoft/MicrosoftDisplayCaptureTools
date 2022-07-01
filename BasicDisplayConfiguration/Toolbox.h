#pragma once
#include "Toolbox.g.h"
#include "ToolboxFactory.g.h"

namespace winrt::BasicDisplayConfiguration::implementation
{
    struct Toolbox : ToolboxT<Toolbox>
    {
        Toolbox();
        Toolbox(winrt::MicrosoftDisplayCaptureTools::Framework::ILogger const& logger);

        hstring Name();
        com_array<hstring> GetSupportedTools();
        winrt::MicrosoftDisplayCaptureTools::ConfigurationTools::IConfigurationTool GetTool(hstring const& toolName);
        void SetConfigData(winrt::Windows::Data::Json::IJsonValue data);

    private:
        const winrt::MicrosoftDisplayCaptureTools::Framework::ILogger m_logger{nullptr};
    };

    struct ToolboxFactory : ToolboxFactoryT<ToolboxFactory>
    {
        ToolboxFactory() = default;

        winrt::MicrosoftDisplayCaptureTools::ConfigurationTools::IConfigurationToolbox CreateConfigurationToolbox(
            winrt::MicrosoftDisplayCaptureTools::Framework::ILogger const& logger);
    };
    }
namespace winrt::BasicDisplayConfiguration::factory_implementation
{
    struct Toolbox : ToolboxT<Toolbox, implementation::Toolbox>
    {
    };
    struct ToolboxFactory : ToolboxFactoryT<ToolboxFactory, implementation::ToolboxFactory>
    {
    };
}
