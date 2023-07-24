export module ResolutionTool;

import "pch.h";
import ToolboxBase;

export namespace winrt::BasicDisplayConfiguration::implementation {

enum class ResolutionToolKind
{
    TargetResolution,
    SourceResolution,
    PlaneResolution
};

struct ResolutionTool : SizeTool<ResolutionTool>
{
    ResolutionTool(ResolutionToolKind kind, winrt::MicrosoftDisplayCaptureTools::Framework::ILogger const& logger);
    winrt::MicrosoftDisplayCaptureTools::ConfigurationTools::IConfigurationToolRequirements Requirements();
    void ApplyToOutput(winrt::MicrosoftDisplayCaptureTools::Display::IDisplayOutput displayOutput);
    void ApplyToPrediction(winrt::MicrosoftDisplayCaptureTools::Display::IDisplayPrediction displayPrediction);

private:
    const ResolutionToolKind m_kind;
    const winrt::MicrosoftDisplayCaptureTools::Framework::ILogger m_logger;

    winrt::event_token m_displaySetupEventToken, m_drawPredictionEventToken;
};
} // namespace winrt::BasicDisplayConfiguration::implementation
