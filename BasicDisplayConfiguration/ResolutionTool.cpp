module ResolutionTool;

import "pch.h";

namespace winrt {
using namespace MicrosoftDisplayCaptureTools::ConfigurationTools;
using namespace MicrosoftDisplayCaptureTools::Display;
using namespace MicrosoftDisplayCaptureTools::Framework;
} // namespace winrt

namespace winrt::BasicDisplayConfiguration::implementation {

static const std::wstring DefaultConfiguration = L"1920x1080";

ResolutionTool::ResolutionTool(ResolutionToolKind kind, winrt::ILogger const& logger) :
    ToolBase::SizeTool<ResolutionTool>(L"Resolution", DefaultConfiguration.c_str(), {L"1600x900", L"1920x1080", L"3840x2160"}, logger),
    m_kind(kind)
{
}

hstring ResolutionTool::Name()
{
    switch (m_kind)
    {
    case ResolutionToolKind::TargetResolution:
        return L"TargetResolution";
    case ResolutionToolKind::SourceResolution:
        return L"SourceResolution";
    case ResolutionToolKind::PlaneResolution:
        return L"PlaneResolution";
    }

    return L"Resolution";
}

void ResolutionTool::ApplyToOutput(IDisplayOutput displayOutput)
{
    m_displaySetupEventToken = displayOutput.DisplaySetupCallback([this](const auto&, IDisplaySetupToolArgs args) {
        auto sourceRes = args.Mode().SourceResolution();
        auto targetRes = args.Mode().TargetResolution();

        if (m_kind == ResolutionToolKind::SourceResolution)
        {
            args.IsModeCompatible(sourceRes == m_configuration && targetRes == m_configuration);
        }
        else if (m_kind == ResolutionToolKind::TargetResolution)
        {
            args.IsModeCompatible(targetRes == m_configuration);
        }
    });

    m_logger.LogNote(L"Registering " + Name() + L": " + ToolConfigConversions::ToConfigString(m_configuration) + L" to be applied.");
}

void ResolutionTool::ApplyToPrediction(IDisplayPrediction displayPrediction)
{
    if (m_kind == ResolutionToolKind::TargetResolution)
    {
        m_drawPredictionEventToken = displayPrediction.DisplaySetupCallback([this](const auto&, IDisplayPredictionData predictionData) {
            predictionData.FrameData().Resolution(m_configuration);
        });
    }
}

winrt::IConfigurationToolRequirements ResolutionTool::Requirements()
{
    return nullptr;
}
} // namespace winrt::BasicDisplayConfiguration::implementation