export module ResolutionTool;

import "pch.h";
import ToolboxBase;

using namespace winrt::MicrosoftDisplayCaptureTools;

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
    ResolutionTool(ResolutionToolKind kind);

    winrt::hstring Name(); // Special overload because this tool can have different names for the different types of ResolutionToolKind

    void ApplyToOutput(Display::IDisplayOutput displayOutput);
    void ApplyToPrediction(ConfigurationTools::IPrediction displayPrediction);

private:
    const ResolutionToolKind m_kind;
};
} // namespace winrt::BasicDisplayConfiguration::implementation
