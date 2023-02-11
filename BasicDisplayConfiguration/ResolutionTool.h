#pragma once
namespace winrt::BasicDisplayConfiguration::implementation
{
	enum class ResolutionToolConfigurations
	{
		w1920h1080,
		w800h600
	};

	struct ResolutionTool : implements<ResolutionTool, winrt::MicrosoftDisplayCaptureTools::ConfigurationTools::IConfigurationTool>
	{
		ResolutionTool(winrt::MicrosoftDisplayCaptureTools::Framework::ILogger const& logger);
		winrt::hstring Name();
		winrt::MicrosoftDisplayCaptureTools::ConfigurationTools::ConfigurationToolCategory Category();
		winrt::MicrosoftDisplayCaptureTools::ConfigurationTools::IConfigurationToolRequirements Requirements();
		winrt::com_array<winrt::hstring> GetSupportedConfigurations();
		winrt::hstring GetDefaultConfiguration();
        winrt::hstring GetConfiguration();
        void SetConfiguration(winrt::hstring configuration);
        void ApplyToOutput(winrt::MicrosoftDisplayCaptureTools::Display::IDisplayOutput displayOutput);
        void ApplyToPrediction(winrt::MicrosoftDisplayCaptureTools::Display::IDisplayPrediction displayPrediction);

	private:
		ResolutionToolConfigurations m_currentConfig;
        static const ResolutionToolConfigurations sc_defaultConfig = ResolutionToolConfigurations::w1920h1080;
        const winrt::MicrosoftDisplayCaptureTools::Framework::ILogger m_logger;

        winrt::event_token m_drawCallbackToken;
	};
}
