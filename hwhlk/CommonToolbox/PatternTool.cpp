#include "pch.h"
#include "winrt\MicrosoftDisplayCaptureTools.Display.h"
#include "winrt\MicrosoftDisplayCaptureTools.ConfigurationTools.h"
#include "PatternTool.h"

namespace winrt
{
	using namespace MicrosoftDisplayCaptureTools::ConfigurationTools;
	using namespace MicrosoftDisplayCaptureTools::Display;
}

namespace winrt::DisplayConfiguration::implementation
{
	std::map<PatternToolConfigurations, winrt::hstring> PatternConfigurationMap
	{
		{ PatternToolConfigurations::Black, L"Black" },
		{ PatternToolConfigurations::White, L"White" },
		{ PatternToolConfigurations::Red,   L"Red"   },
		{ PatternToolConfigurations::Green, L"Green" },
		{ PatternToolConfigurations::Blue,  L"Blue"  }
	};

	PatternTool::PatternTool() : m_currentConfig(sc_defaultConfig)
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
		for (auto config : PatternConfigurationMap)
		{
			configs.push_back(config.second);
		}

		return com_array<hstring>(configs);
	}

	hstring PatternTool::GetDefaultConfiguration()
	{
		return PatternConfigurationMap[sc_defaultConfig];
	}

	void PatternTool::SetConfiguration(hstring configuration)
	{
		for (auto config : PatternConfigurationMap)
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
	}
}