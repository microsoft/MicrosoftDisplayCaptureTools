import "pch.h";
#include "Toolbox.h"
#include "Toolbox.g.cpp"
#include "ToolboxFactory.g.cpp"

#include "BasePlanePattern.h"
import ResolutionTool;
import RefreshRateTool;
#include "PixelFormatTool.h"

import PredictionRenderer;

using namespace winrt::MicrosoftDisplayCaptureTools::Framework::Helpers;
namespace winrt
{
    using namespace winrt::Windows::Data::Json;
    using namespace winrt::MicrosoftDisplayCaptureTools::ConfigurationTools;
    using namespace winrt::MicrosoftDisplayCaptureTools::Framework;
}

namespace winrt::BasicDisplayConfiguration::implementation
{
    winrt::IConfigurationToolbox ToolboxFactory::CreateConfigurationToolbox()
    {
        return winrt::make<Toolbox>();
    }

    Toolbox::Toolbox()
    {
    }

    MicrosoftDisplayCaptureTools::ConfigurationTools::IPrediction Toolbox::CreatePrediction()
    {
        return winrt::make<PredictionRenderer::Prediction>();
    }

    enum class Tools
    {
        Pattern,
        TargetResolution,
        SourceResolution,
        PlaneResolution,
        RefreshRate,
        SurfacePixelFormat,
        SourcePixelFormat
    };

    std::map<hstring, Tools> MapNameToTool =
    {
        {L"Pattern", Tools::Pattern},
        {L"TargetResolution", Tools::TargetResolution},
        {L"SourceResolution", Tools::SourceResolution},
        {L"PlaneResolution", Tools::PlaneResolution},
        {L"RefreshRate", Tools::RefreshRate},
        {L"SurfacePixelFormat", Tools::SurfacePixelFormat},
        {L"SourcePixelFormat", Tools::SourcePixelFormat}
    };

    hstring Toolbox::Name()
    {
        return L"BasicDisplayConfiguration";
    }
    com_array<hstring> Toolbox::GetSupportedTools()
    {
        auto toolNames = std::vector<hstring>();
        for (auto tool : MapNameToTool)
        {
            toolNames.push_back(tool.first);
        }

        return com_array<hstring>(toolNames);
    }
    IConfigurationTool Toolbox::GetTool(hstring const& toolName)
    {
        switch (MapNameToTool[toolName])
        {
        case Tools::Pattern:
            return winrt::make<BasePlanePattern>();
        case Tools::RefreshRate:
            return winrt::make<RefreshRateTool>();
        case Tools::TargetResolution:
            return winrt::make<ResolutionTool>(ResolutionToolKind::TargetResolution);
        case Tools::SourceResolution:
            return winrt::make<ResolutionTool>(ResolutionToolKind::SourceResolution);
        case Tools::PlaneResolution:
            return winrt::make<ResolutionTool>(ResolutionToolKind::PlaneResolution);
        case Tools::SourcePixelFormat:
            return winrt::make<PixelFormatTool>(PixelFormatToolKind::SourcePixelFormat);
        case Tools::SurfacePixelFormat:
            return winrt::make<PixelFormatTool>(PixelFormatToolKind::PlanePixelFormat);
        }

        // The caller has asked for a tool that is not exposed from this toolbox
        Logger().LogAssert(Name() + L"::GetTool was called with invalid tool name: " + toolName);
        throw winrt::hresult_invalid_argument();
    }

    void Toolbox::SetConfigData(IJsonValue data)
    {
    }
}
