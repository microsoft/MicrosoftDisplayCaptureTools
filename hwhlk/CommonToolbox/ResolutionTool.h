#pragma once
namespace winrt::DisplayConfiguration::implementation
{
	enum class ResolutionToolConfigurations
	{
		w640h360,
		w1280h720,
		w1920h1080
	};

	struct ResolutionTool : implements<ResolutionTool, winrt::MicrosoftDisplayCaptureTools::ConfigurationTools::IConfigurationTool>
	{
		ResolutionTool();
		winrt::hstring Name();
		winrt::MicrosoftDisplayCaptureTools::ConfigurationTools::ConfigurationToolCategory Category();
		winrt::MicrosoftDisplayCaptureTools::ConfigurationTools::IConfigurationToolRequirements Requirements();
		winrt::com_array<winrt::hstring> GetSupportedConfigurations();
		winrt::hstring GetDefaultConfiguration();
		void SetConfiguration(winrt::hstring configuration);
		void Apply(winrt::MicrosoftDisplayCaptureTools::Display::IDisplayEngine reference);

	private:
		ResolutionToolConfigurations m_currentConfig;
		static const ResolutionToolConfigurations sc_defaultConfig = ResolutionToolConfigurations::w1920h1080;
	};
}
