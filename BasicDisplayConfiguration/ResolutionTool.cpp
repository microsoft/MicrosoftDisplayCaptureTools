module ResolutionTool;

import "pch.h";

namespace winrt {
using namespace MicrosoftDisplayCaptureTools::ConfigurationTools;
using namespace MicrosoftDisplayCaptureTools::Display;
using namespace MicrosoftDisplayCaptureTools::Framework;
} // namespace winrt

namespace winrt::BasicDisplayConfiguration::implementation {

static const std::wstring DefaultConfiguration = L"1920x1080";

std::map<std::wstring, Windows::Graphics::SizeInt32> ConfigurationMap{
    {L"1600x900", {1600, 900}}, {L"1920x1080", {1920, 1080}}, {L"3840x2160", {3840, 2160}}};

ResolutionTool::ResolutionTool(ResolutionToolKind kind, winrt::ILogger const& logger) :
    __super()
    m_kind(kind), m_currentConfig(DefaultConfiguration), m_logger(logger)
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

ConfigurationToolCategory ResolutionTool::Category()
{
    return ConfigurationToolCategory::DisplaySetup;
}

void ResolutionTool::ApplyToOutput(IDisplayOutput displayOutput)
{
    m_displaySetupEventToken = displayOutput.DisplaySetupCallback([this](const auto&, IDisplaySetupToolArgs args) {
        auto sourceRes = args.Mode().SourceResolution();
        auto targetRes = args.Mode().TargetResolution();
        auto& configRes = ConfigurationMap[m_currentConfig];

        if (m_kind == ResolutionToolKind::SourceResolution)
        {
            args.IsModeCompatible(sourceRes == configRes && targetRes == configRes);
        }
        else if (m_kind == ResolutionToolKind::TargetResolution)
        {
            args.IsModeCompatible(targetRes == configRes);
        }
    });

    m_logger.LogNote(L"Registering " + Name() + L": " + m_currentConfig + L" to be applied.");
}

void ResolutionTool::ApplyToPrediction(IDisplayPrediction displayPrediction)
{
    if (m_kind == ResolutionToolKind::TargetResolution)
    {
        m_drawPredictionEventToken = displayPrediction.DisplaySetupCallback([this](const auto&, IDisplayPredictionData predictionData) {
            predictionData.FrameData().Resolution(ConfigurationMap[m_currentConfig]);
        });
    }
}
} // namespace winrt::BasicDisplayConfiguration::implementation