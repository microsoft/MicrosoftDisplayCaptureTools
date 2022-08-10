#pragma once
namespace winrt::BasicDisplayConfiguration::implementation
{
	enum class RefreshRateToolConfigurations
	{
		r60,
		r75,
	};

	struct RefreshRateTool : implements<RefreshRateTool, winrt::MicrosoftDisplayCaptureTools::ConfigurationTools::IConfigurationTool>
	{
        RefreshRateTool(winrt::MicrosoftDisplayCaptureTools::Framework::ILogger const& logger);
		winrt::hstring Name();
		winrt::MicrosoftDisplayCaptureTools::ConfigurationTools::ConfigurationToolCategory Category();
		winrt::MicrosoftDisplayCaptureTools::ConfigurationTools::IConfigurationToolRequirements Requirements();
		winrt::com_array<winrt::hstring> GetSupportedConfigurations();
		winrt::hstring GetDefaultConfiguration();
        winrt::hstring GetConfiguration();
		void SetConfiguration(winrt::hstring configuration);
		void Apply(winrt::MicrosoftDisplayCaptureTools::Display::IDisplayOutput reference);

	private:
		RefreshRateToolConfigurations m_currentConfig;
        static const RefreshRateToolConfigurations sc_defaultConfig = RefreshRateToolConfigurations::r60;
        const winrt::MicrosoftDisplayCaptureTools::Framework::ILogger m_logger;
	};
}
