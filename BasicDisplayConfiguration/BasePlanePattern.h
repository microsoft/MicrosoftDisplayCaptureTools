#pragma once

namespace winrt::BasicDisplayConfiguration::implementation {

struct BasePlanePattern : implements<BasePlanePattern, winrt::MicrosoftDisplayCaptureTools::ConfigurationTools::IConfigurationTool>
{
    BasePlanePattern();
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
    void RenderPatternToPlane(const winrt::Microsoft::Graphics::Canvas::CanvasDrawingSession& drawingSession, float width, float height);

private:
    std::wstring m_currentConfig;

    winrt::event_token m_drawOutputEventToken, m_drawPredictionEventToken;
};

} // namespace winrt::BasicDisplayConfiguration::implementation
