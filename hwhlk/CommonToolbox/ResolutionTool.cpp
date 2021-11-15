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
		{ ResolutionToolConfigurations::w640h360,   L"640x360" },
		{ ResolutionToolConfigurations::w1280h720,  L"1280x720" },
		{ ResolutionToolConfigurations::w1920h1080, L"1920x1080" }
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

	ConfigurationToolRequirements ResolutionTool::Requirements()
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
		case ResolutionToolConfigurations::w640h360:
			displayProperties.Resolution({ 640, 360 });
			break;
		case ResolutionToolConfigurations::w1280h720:
			displayProperties.Resolution({ 1280, 720 });
			break;
		case ResolutionToolConfigurations::w1920h1080:
			displayProperties.Resolution({ 1920, 1080 });
			break;
		}

	}
}