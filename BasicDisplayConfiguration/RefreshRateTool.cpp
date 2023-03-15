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
	static const std::wstring DefaultConfiguration = L"60hz";
	std::map<std::wstring, double> ConfigurationMap
	{
		{ L"60hz", 60. },
		{ L"75hz", 75. }
	};

	RefreshRateTool::RefreshRateTool(winrt::ILogger const& logger) : 
		m_currentConfig(DefaultConfiguration),
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
        for (auto&& config : ConfigurationMap)
        {
            hstring configName(config.first);
            configs.push_back(configName);
        }

        return com_array<hstring>(configs);
	}

	hstring RefreshRateTool::GetDefaultConfiguration()
    {
        hstring defaultConfig(DefaultConfiguration);
        return defaultConfig;
	}

	hstring RefreshRateTool::GetConfiguration()
    {
        hstring currentConfig(m_currentConfig);
        return currentConfig;
	}

	void RefreshRateTool::SetConfiguration(hstring configuration)
    {
        if (ConfigurationMap.find(configuration.c_str()) == ConfigurationMap.end())
        {
            // An invalid configuration was asked for
            m_logger.LogError(L"An invalid configuration was requested: " + configuration);

            throw winrt::hresult_invalid_argument();
        }

        m_currentConfig = configuration.c_str();
	}

	void RefreshRateTool::ApplyToOutput(IDisplayOutput displayOutput)
    {
        constexpr double sc_refreshRateEpsilon = 0.00000000001;

        m_displaySetupEventToken = displayOutput.DisplaySetupCallback([this](const auto&, IDisplaySetupToolArgs args) 
		{
            double presentationRate = static_cast<double>(args.Mode().PresentationRate().VerticalSyncRate.Numerator) /
                                      static_cast<double>(args.Mode().PresentationRate().VerticalSyncRate.Denominator);

            args.IsModeCompatible(fabs(presentationRate - ConfigurationMap[m_currentConfig]) < sc_refreshRateEpsilon);
        });

        m_logger.LogNote(L"Registering " + Name() + L": " + m_currentConfig + L" to be applied.");
	}

	void RefreshRateTool::ApplyToPrediction(IDisplayPrediction displayPrediction)
    {
        // This tool doesn't currently matter to the output
	}
}