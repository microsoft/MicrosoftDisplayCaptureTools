#include "pch.h"
#include "ResolutionTool.h"

namespace winrt
{
	using namespace MicrosoftDisplayCaptureTools::ConfigurationTools;
	using namespace MicrosoftDisplayCaptureTools::Display;
	using namespace MicrosoftDisplayCaptureTools::Framework;
}

namespace winrt::BasicDisplayConfiguration::implementation
{
	std::map<ResolutionToolConfigurations, winrt::hstring> ConfigurationMap
	{
		{ ResolutionToolConfigurations::w1920h1080, L"1920x1080" },
		{ ResolutionToolConfigurations::w800h600, L"800x600" }
	};

	ResolutionTool::ResolutionTool(winrt::ILogger const& logger) : 
		m_currentConfig(sc_defaultConfig), 
		m_logger(logger)
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
		for (auto config : ConfigurationMap)
		{
			configs.push_back(config.second);
		}

		return com_array<hstring>(configs);
	}

	hstring ResolutionTool::GetDefaultConfiguration()
	{
		return ConfigurationMap[sc_defaultConfig];
	}

    hstring ResolutionTool::GetConfiguration()
    {
        return ConfigurationMap[m_currentConfig];
	}

	void ResolutionTool::SetConfiguration(hstring configuration)
	{
		for (auto config : ConfigurationMap)
		{
			if (config.second == configuration)
			{
				m_currentConfig = config.first;
				return;
			}
		}

		// An invalid configuration was asked for
        m_logger.LogError(L"An invalid configuration was requested: " + configuration);

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

		m_logger.LogNote(L"Using " + Name() + L": " + ConfigurationMap[m_currentConfig]);
	}
}