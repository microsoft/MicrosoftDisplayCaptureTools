#include "pch.h"
#include "RefreshRateTool.h"

namespace winrt
{
	using namespace MicrosoftDisplayCaptureTools::ConfigurationTools;
	using namespace MicrosoftDisplayCaptureTools::Display;
	using namespace MicrosoftDisplayCaptureTools::Framework;
}

namespace winrt::BasicDisplayConfiguration::implementation
{
	std::map<RefreshRateToolConfigurations, winrt::hstring> ConfigurationMap
	{
		{ RefreshRateToolConfigurations::r60, L"60hz" },
		{ RefreshRateToolConfigurations::r75, L"75hz" }
	};

	RefreshRateTool::RefreshRateTool(winrt::ILogger const& logger) : 
		m_currentConfig(sc_defaultConfig),
		m_logger(logger)
	{
	}

	hstring RefreshRateTool::Name()
	{
		return L"RefreshRate";
	}

	ConfigurationToolCategory RefreshRateTool::Category()
	{
		return ConfigurationToolCategory::DisplaySetup;
	}

	IConfigurationToolRequirements RefreshRateTool::Requirements()
	{
		return nullptr;
	}

	com_array<hstring> RefreshRateTool::GetSupportedConfigurations()
	{
		std::vector<hstring> configs;
		for (auto config : ConfigurationMap)
		{
			configs.push_back(config.second);
		}

		return com_array<hstring>(configs);
	}

	hstring RefreshRateTool::GetDefaultConfiguration()
	{
		return ConfigurationMap[sc_defaultConfig];
	}

	hstring RefreshRateTool::GetConfiguration()
	{
        return ConfigurationMap[m_currentConfig];
	}

	void RefreshRateTool::SetConfiguration(hstring configuration)
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

	void RefreshRateTool::Apply(IDisplayEngine reference)
	{
		auto displayProperties = reference.GetProperties();

		switch (m_currentConfig)
		{
		case RefreshRateToolConfigurations::r60:
			displayProperties.RefreshRate(60.);
			break;
		case RefreshRateToolConfigurations::r75:
			displayProperties.RefreshRate(75.);
			break;
        }

        m_logger.LogNote(L"Using " + Name() + L": " + ConfigurationMap[m_currentConfig]);
	}
}