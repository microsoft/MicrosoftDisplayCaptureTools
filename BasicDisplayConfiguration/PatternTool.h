#pragma once

namespace winrt::BasicDisplayConfiguration::implementation {

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
    void ApplyToOutput(winrt::MicrosoftDisplayCaptureTools::Display::IDisplayOutput displayOutput);
    void ApplyToPrediction(winrt::MicrosoftDisplayCaptureTools::ConfigurationTools::IPrediction displayPrediction);

private:
    void RenderPatternToPlane(const winrt::Microsoft::Graphics::Canvas::CanvasDrawingSession& drawingSession, uint32_t width, uint32_t height);

private:
    std::wstring m_currentConfig;

    const winrt::MicrosoftDisplayCaptureTools::Framework::ILogger m_logger;

    winrt::event_token m_drawOutputEventToken, m_drawPredictionEventToken;
};

} // namespace winrt::BasicDisplayConfiguration::implementation
