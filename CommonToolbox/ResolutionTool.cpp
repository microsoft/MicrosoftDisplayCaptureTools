#include "pch.h"
#include "winrt\MicrosoftDisplayCaptureTools.Display.h"
#include "winrt\MicrosoftDisplayCaptureTools.ConfigurationTools.h"
#include "ResolutionTool.h"

namespace winrt
{
	using namespace MicrosoftDisplayCaptureTools::ConfigurationTools;
	using namespace MicrosoftDisplayCaptureTools::Display;
}

namespace winrt::DisplayConfiguration::implementation
{
	std::map<ResolutionToolConfigurations, winrt::hstring> ResolutionConfigurationMap
	{
		{ ResolutionToolConfigurations::w1920h1080, L"1920x1080" },
		{ ResolutionToolConfigurations::w800h600, L"800x600" }
	};

	ResolutionTool::ResolutionTool() : m_currentConfig(sc_defaultConfig)
	{
	}

	hstring ResolutionTool::Name()
	{
		return L"Resolution";
	}

	ConfigurationToolCategory ResolutionTool::Category()
	{
		return ConfigurationToolCategory::DisplaySetup;
	}

	IConfigurationToolRequirements ResolutionTool::Requirements()
	{
		return nullptr;
	}

	com_array<hstring> ResolutionTool::GetSupportedConfigurations()
	{
		std::vector<hstring> configs;
		for (auto config : ResolutionConfigurationMap)
		{
			configs.push_back(config.second);
		}

		return com_array<hstring>(configs);
	}

	hstring ResolutionTool::GetDefaultConfiguration()
	{
		return ResolutionConfigurationMap[sc_defaultConfig];
	}

    hstring ResolutionTool::GetConfiguration()
    {
        return ResolutionConfigurationMap[m_currentConfig];
	}

	void ResolutionTool::SetConfiguration(hstring configuration)
	{
		for (auto config : ResolutionConfigurationMap)
		{
			if (config.second == configuration)
			{
				m_currentConfig = config.first;
				return;
			}
		}

		// An invalid configuration was asked for
		// TODO: log this case
		throw winrt::hresult_invalid_argument();
	}

	void ResolutionTool::Apply(IDisplayEngine reference)
	{
		auto displayProperties = reference.GetProperties();

		switch (m_currentConfig)
		{
		case ResolutionToolConfigurations::w1920h1080:
			displayProperties.Resolution({ 1920, 1080 });
			break;
		case ResolutionToolConfigurations::w800h600:
			displayProperties.Resolution({ 800, 600 });
			break;
		}

	}
}