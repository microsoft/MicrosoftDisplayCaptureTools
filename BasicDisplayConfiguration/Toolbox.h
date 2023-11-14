#pragma once
#include "Toolbox.g.h"
#include "ToolboxFactory.g.h"

namespace winrt::BasicDisplayConfiguration::implementation
{
    struct Toolbox : ToolboxT<Toolbox>
    {
        Toolbox();

        // MicrosoftDisplayCaptureTools.ConfigurationTools.IConfigurationToolbox
        hstring Name();
        com_array<hstring> GetSupportedTools();
        winrt::MicrosoftDisplayCaptureTools::ConfigurationTools::IConfigurationTool GetTool(hstring const& toolName);
        void SetConfigData(winrt::Windows::Data::Json::IJsonValue data);
        
        MicrosoftDisplayCaptureTools::ConfigurationTools::IPrediction CreatePrediction();

        MicrosoftDisplayCaptureTools::Framework::Version Version()
        {
            return MicrosoftDisplayCaptureTools::Framework::Version(0, 1, 0);
        };


    private:
        winrt::Windows::Foundation::Collections::IMap<winrt::hstring, winrt::Windows::Foundation::IInspectable> m_runtimeSettings;
    };

    struct ToolboxFactory : ToolboxFactoryT<ToolboxFactory>
    {
        ToolboxFactory() = default;

        winrt::MicrosoftDisplayCaptureTools::ConfigurationTools::IConfigurationToolbox CreateConfigurationToolbox();
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
