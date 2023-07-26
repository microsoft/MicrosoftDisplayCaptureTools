export module RefreshRateTool;

import "pch.h";
import ToolboxBase;

using namespace winrt::MicrosoftDisplayCaptureTools::Framework;
using namespace winrt::MicrosoftDisplayCaptureTools::Display;
using namespace winrt::MicrosoftDisplayCaptureTools::ConfigurationTools;

export namespace winrt::BasicDisplayConfiguration::implementation {

    struct RefreshRateTool : ToolBase::IntTool<RefreshRateTool>
    {
        RefreshRateTool(ILogger const& logger);

        IConfigurationToolRequirements Requirements();
        void ApplyToOutput(IDisplayOutput displayOutput);
        void ApplyToPrediction(IDisplayPrediction displayPrediction);

    private:
        winrt::event_token m_displaySetupEventToken, m_drawPredictionEventToken;
    };
} // namespace winrt::BasicDisplayConfiguration::implementation
