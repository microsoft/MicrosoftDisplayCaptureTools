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
	static const std::wstring DefaultConfiguration = L"1920x1080";
	std::map<std::wstring, Windows::Graphics::SizeInt32> ConfigurationMap
	{
		{ L"1024x768",  {1024, 768 } },
		{ L"1920x1080", {1920, 1080} },
		{ L"3840x2160", {3840, 2160} }
	};

	ResolutionTool::ResolutionTool(winrt::ILogger const& logger) : 
		m_currentConfig(DefaultConfiguration), 
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
        for (auto&& config : ConfigurationMap)
        {
            hstring configName(config.first);
            configs.push_back(configName);
        }

        return com_array<hstring>(configs);
	}

	hstring ResolutionTool::GetDefaultConfiguration()
    {
        hstring defaultConfig(DefaultConfiguration);
        return defaultConfig;
	}

    hstring ResolutionTool::GetConfiguration()
    {
        hstring currentConfig(m_currentConfig);
        return currentConfig;
	}

	void ResolutionTool::SetConfiguration(hstring configuration)
    {
        if (ConfigurationMap.find(configuration.c_str()) == ConfigurationMap.end())
        {
            // An invalid configuration was asked for
            m_logger.LogError(L"An invalid configuration was requested: " + configuration);

            throw winrt::hresult_invalid_argument();
        }

        m_currentConfig = configuration.c_str();
	}

	void ResolutionTool::ApplyToOutput(IDisplayOutput displayOutput)
    {
        m_displaySetupEventToken = displayOutput.DisplaySetupCallback([this](const auto&, IDisplaySetupToolArgs args) 
		{
            auto sourceRes = args.Mode().SourceResolution();
            auto targetRes = args.Mode().TargetResolution();

            auto& configRes = ConfigurationMap[m_currentConfig];
            args.IsModeCompatible(sourceRes == configRes && targetRes == configRes);
        });

        m_logger.LogNote(L"Registering " + Name() + L": " + m_currentConfig + L" to be applied.");
	}

	void ResolutionTool::ApplyToPrediction(IDisplayPrediction displayPrediction)
    {
        m_drawPredictionEventToken = displayPrediction.DisplaySetupCallback([this](const auto&, IDisplayPredictionData predictionData) 
		{ 
            predictionData.FrameData().Resolution(ConfigurationMap[m_currentConfig]);
		});
	}
}