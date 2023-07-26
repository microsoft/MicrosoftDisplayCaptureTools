export module RefreshRateTool;

import "pch.h";
import ToolboxBase;

using namespace winrt::MicrosoftDisplayCaptureTools;

export namespace winrt::BasicDisplayConfiguration::implementation {

    struct RefreshRateTool : ToolBase::IntTool<RefreshRateTool>
    {
        RefreshRateTool(Framework::ILogger const& logger);

        void ApplyToOutput(Display::IDisplayOutput displayOutput);
    };
} // namespace winrt::BasicDisplayConfiguration::implementation
