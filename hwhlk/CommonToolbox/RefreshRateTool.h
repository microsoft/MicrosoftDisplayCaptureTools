#pragma once
namespace winrt::DisplayConfiguration::implementation
{
	enum class RefreshRateToolConfigurations
	{
		r60,
		r50,
	};

	struct RefreshRateTool : implements<RefreshRateTool, winrt::MicrosoftDisplayCaptureTools::ConfigurationTools::IConfigurationTool>
	{
		RefreshRateTool();
		winrt::hstring Name();
		winrt::MicrosoftDisplayCaptureTools::ConfigurationTools::ConfigurationToolCategory Category();
		winrt::MicrosoftDisplayCaptureTools::ConfigurationTools::ConfigurationToolRequirements Requirements();
		winrt::com_array<winrt::hstring> GetSupportedConfigurations();
		winrt::hstring GetDefaultConfiguration();
		void SetConfiguration(winrt::hstring configuration);
		void Apply(winrt::MicrosoftDisplayCaptureTools::Display::IDisplayEngine reference);

	private:
		RefreshRateToolConfigurations m_currentConfig;
		static const RefreshRateToolConfigurations sc_defaultConfig = RefreshRateToolConfigurations::r60;
	};
}
