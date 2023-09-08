module ResolutionTool;
import PredictionRenderer;

import "pch.h";

namespace winrt {
using namespace MicrosoftDisplayCaptureTools::ConfigurationTools;
using namespace MicrosoftDisplayCaptureTools::Display;
using namespace MicrosoftDisplayCaptureTools::Framework;
} // namespace winrt

namespace winrt::BasicDisplayConfiguration::implementation {

static const winrt::hstring DefaultConfiguration = L"1920x1080";

ResolutionTool::ResolutionTool(ResolutionToolKind kind, winrt::ILogger const& logger) :
    ToolBase::SizeTool<ResolutionTool>(L"Resolution", DefaultConfiguration, {L"1600x900", L"1920x1080", L"3840x2160"}, logger),
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
    m_eventTokens[L"DisplaySetup"] = displayOutput.DisplaySetupCallback([this](const auto&, IDisplaySetupToolArgs args) {
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

    m_logger.LogNote(L"Registering " + Name() + L": " + GetCurrentConfigurationString() + L" to be applied.");
}

void ResolutionTool::ApplyToPrediction(IPrediction displayPrediction)
{
    if (m_kind == ResolutionToolKind::TargetResolution)
    {
        m_eventTokens[L"PredictionEvent"] = displayPrediction.DisplaySetupCallback([this](const auto&, IPredictionData predictionData) 
        {
            auto prediction = predictionData.as<PredictionRenderer::PredictionData>();

            for (auto& frame : prediction->Frames())
            {
                frame.TargetModeSize.Width = static_cast<float>(m_configuration.Width);
                frame.TargetModeSize.Height = static_cast<float>(m_configuration.Height);
                frame.SourceModeSize.Width = static_cast<float>(m_configuration.Width);
                frame.SourceModeSize.Height = static_cast<float>(m_configuration.Height);
            }
        });
    }
}
} // namespace winrt::BasicDisplayConfiguration::implementation