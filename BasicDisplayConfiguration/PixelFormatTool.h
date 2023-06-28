#pragma once
namespace winrt::BasicDisplayConfiguration::implementation {

enum class PixelFormatToolKind
{
    SourcePixelFormat,
    PlanePixelFormat,
};

struct PixelFormatTool : implements<PixelFormatTool, winrt::MicrosoftDisplayCaptureTools::ConfigurationTools::IConfigurationTool>
{
    PixelFormatTool(PixelFormatToolKind kind, winrt::MicrosoftDisplayCaptureTools::Framework::ILogger const& logger);
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
    PixelFormatToolKind m_kind;
    std::wstring m_currentConfig;
    const winrt::MicrosoftDisplayCaptureTools::Framework::ILogger m_logger;

    winrt::event_token m_displaySetupEventToken, m_drawPredictionEventToken;
};
} // namespace winrt::BasicDisplayConfiguration::implementation
