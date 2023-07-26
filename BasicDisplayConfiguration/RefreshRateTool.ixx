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

        void ApplyToOutput(IDisplayOutput displayOutput);
    };
} // namespace winrt::BasicDisplayConfiguration::implementation
