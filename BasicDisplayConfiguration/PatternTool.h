#pragma once
namespace winrt::BasicDisplayConfiguration::implementation
{
	enum class PatternToolConfigurations
	{
		Black,
		White,
		Red,
		Green,
		Blue
	};

	struct PatternTool : implements<PatternTool, winrt::MicrosoftDisplayCaptureTools::ConfigurationTools::IConfigurationTool>
	{
        PatternTool(winrt::MicrosoftDisplayCaptureTools::Framework::ILogger const& logger);
		winrt::hstring Name();
		winrt::MicrosoftDisplayCaptureTools::ConfigurationTools::ConfigurationToolCategory Category();
		winrt::MicrosoftDisplayCaptureTools::ConfigurationTools::IConfigurationToolRequirements Requirements();
		winrt::com_array<winrt::hstring> GetSupportedConfigurations();
		winrt::hstring GetDefaultConfiguration();
        winrt::hstring GetConfiguration();
		void SetConfiguration(winrt::hstring configuration);
		void Apply(winrt::MicrosoftDisplayCaptureTools::Display::IDisplayOutput reference);

	private:
		PatternToolConfigurations m_currentConfig;
        static const PatternToolConfigurations sc_defaultConfig = PatternToolConfigurations::Green;

		const winrt::MicrosoftDisplayCaptureTools::Framework::ILogger m_logger;
	};
}
