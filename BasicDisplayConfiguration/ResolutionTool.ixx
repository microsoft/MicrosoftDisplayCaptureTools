export module ResolutionTool;

import "pch.h";
import ToolboxBase;

using namespace winrt::MicrosoftDisplayCaptureTools::Framework;
using namespace winrt::MicrosoftDisplayCaptureTools::Display;
using namespace winrt::MicrosoftDisplayCaptureTools::ConfigurationTools;

export namespace winrt::BasicDisplayConfiguration::implementation {

enum class ResolutionToolKind
{
    TargetResolution,
    SourceResolution,
    PlaneResolution
};

struct ResolutionTool
    : ToolBase::SizeTool<ResolutionTool>
{
    ResolutionTool(ResolutionToolKind kind, ILogger const& logger);

    winrt::hstring Name(); // Special overload because this tool can have different names for the different types of ResolutionToolKind

    void ApplyToOutput(IDisplayOutput displayOutput);
    void ApplyToPrediction(IDisplayPrediction displayPrediction);

private:
    const ResolutionToolKind m_kind;
};
} // namespace winrt::BasicDisplayConfiguration::implementation
