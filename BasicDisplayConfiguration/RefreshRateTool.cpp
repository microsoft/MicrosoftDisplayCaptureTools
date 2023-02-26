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

	void RefreshRateTool::ApplyToOutput(IDisplayOutput displayOutput)
    {
        constexpr double sc_refreshRateEpsilon = 0.00000000001;

        m_displaySetupEventToken = displayOutput.DisplaySetupCallback([this](const auto&, IDisplaySetupToolArgs args) 
		{
            double presentationRate = static_cast<double>(args.Mode().PresentationRate().VerticalSyncRate.Numerator) /
                                      static_cast<double>(args.Mode().PresentationRate().VerticalSyncRate.Denominator);

            switch (m_currentConfig)
            {
            case RefreshRateToolConfigurations::r60:
                args.IsModeCompatible(fabs(presentationRate - 60.0) < sc_refreshRateEpsilon);
                return;
            case RefreshRateToolConfigurations::r75:
                args.IsModeCompatible(fabs(presentationRate - 75.0) < sc_refreshRateEpsilon);
                return;
            default:
                m_logger.LogError(Name() + L" was set to use an invalid configuration option.");
            }
        });

        m_logger.LogNote(L"Registering " + Name() + L": " + ConfigurationMap[m_currentConfig] + L" to be applied.");
	}

	void RefreshRateTool::ApplyToPrediction(IDisplayPrediction displayPrediction)
    {
        // This tool doesn't currently matter to the output
	}
}