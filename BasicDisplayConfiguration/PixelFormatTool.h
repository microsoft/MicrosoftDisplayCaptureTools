#pragma once
namespace winrt::BasicDisplayConfiguration::implementation {
enum class PixelFormatToolConfigurations
{
    R8G8B8A8UIntNormalized_Interlaced,
};

struct PixelFormatTool : implements<PixelFormatTool, winrt::MicrosoftDisplayCaptureTools::ConfigurationTools::IConfigurationTool>
{
    PixelFormatTool(winrt::MicrosoftDisplayCaptureTools::Framework::ILogger const& logger);
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
    PixelFormatToolConfigurations m_currentConfig;
    static const PixelFormatToolConfigurations sc_defaultConfig = PixelFormatToolConfigurations::R8G8B8A8UIntNormalized_Interlaced;
    const winrt::MicrosoftDisplayCaptureTools::Framework::ILogger m_logger;

    winrt::event_token m_displaySetupEventToken;
};
} // namespace winrt::BasicDisplayConfiguration::implementation
