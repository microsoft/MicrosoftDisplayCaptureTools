#include "pch.h"
#include "PatternTool.h"

namespace winrt
{
	using namespace MicrosoftDisplayCaptureTools::ConfigurationTools;
	using namespace MicrosoftDisplayCaptureTools::Display;
	using namespace MicrosoftDisplayCaptureTools::Framework;
}

namespace winrt::BasicDisplayConfiguration::implementation
{
	std::map<PatternToolConfigurations, winrt::hstring> ConfigurationMap
	{
		{ PatternToolConfigurations::Black, L"Black" },
		{ PatternToolConfigurations::White, L"White" },
		{ PatternToolConfigurations::Red,   L"Red"   },
		{ PatternToolConfigurations::Green, L"Green" },
		{ PatternToolConfigurations::Blue,  L"Blue"  }
	};

	PatternTool::PatternTool(winrt::ILogger const& logger) :
		m_currentConfig(sc_defaultConfig),
		m_logger(logger)
	{
	}

	hstring PatternTool::Name()
	{
		return L"Pattern";
	}

	ConfigurationToolCategory PatternTool::Category()
	{
		return ConfigurationToolCategory::Render;
	}

	IConfigurationToolRequirements PatternTool::Requirements()
	{
		return nullptr;
	}

	com_array<hstring> PatternTool::GetSupportedConfigurations()
	{
		std::vector<hstring> configs;
		for (auto config : ConfigurationMap)
		{
			configs.push_back(config.second);
		}

		return com_array<hstring>(configs);
	}

	hstring PatternTool::GetDefaultConfiguration()
	{
		return ConfigurationMap[sc_defaultConfig];
	}

	hstring PatternTool::GetConfiguration()
    {
        return ConfigurationMap[m_currentConfig];
	}

	void PatternTool::SetConfiguration(hstring configuration)
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

	void PatternTool::Apply(IDisplayEngine reference)
	{
		auto displayProperties = reference.GetProperties();
		auto planeProperties = displayProperties.GetPlaneProperties()[0];

		switch (m_currentConfig)
		{
		case PatternToolConfigurations::Black:
			planeProperties.ClearColor({ 0.f,0.f,0.f });
			break;
		case PatternToolConfigurations::White:
			planeProperties.ClearColor({ 1.f,1.f,1.f });
			break;
		case PatternToolConfigurations::Red:
			planeProperties.ClearColor({ 1.f,0.f,0.f });
			break;
		case PatternToolConfigurations::Green:
			planeProperties.ClearColor({ 0.f,1.f,0.f });
			break;
		case PatternToolConfigurations::Blue:
			planeProperties.ClearColor({ 0.f,0.f,1.f });
			break;
		}

        m_logger.LogNote(L"Using " + Name() + L": " + ConfigurationMap[m_currentConfig]);
	}
}