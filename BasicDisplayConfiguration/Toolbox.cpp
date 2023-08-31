import "pch.h";
#include "Toolbox.h"
#include "Toolbox.g.cpp"
#include "ToolboxFactory.g.cpp"

#include "BasePlanePattern.h"
import ResolutionTool;
import RefreshRateTool;
#include "PixelFormatTool.h"

import PredictionRenderer;

namespace winrt
{
    using namespace winrt::Windows::Data::Json;
    using namespace winrt::MicrosoftDisplayCaptureTools::ConfigurationTools;
    using namespace winrt::MicrosoftDisplayCaptureTools::Framework;
}

namespace winrt::BasicDisplayConfiguration::implementation
{
    winrt::IConfigurationToolbox ToolboxFactory::CreateConfigurationToolbox(winrt::ILogger const& logger)
    {
        return winrt::make<Toolbox>(logger);
    }

    Toolbox::Toolbox()
    {
        // Throw - callers should explicitly instantiate through the factory
        throw winrt::hresult_illegal_method_call();
    }

    Toolbox::Toolbox(winrt::ILogger const& logger) : m_logger(logger)
    {
    }

    MicrosoftDisplayCaptureTools::ConfigurationTools::IPrediction Toolbox::CreatePrediction()
    {
        return winrt::make<Prediction>(m_logger);
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
            return winrt::make<BasePlanePattern>(m_logger);
        case Tools::RefreshRate:
            return winrt::make<RefreshRateTool>(m_logger);
        case Tools::TargetResolution:
            return winrt::make<ResolutionTool>(ResolutionToolKind::TargetResolution, m_logger);
        case Tools::SourceResolution:
            return winrt::make<ResolutionTool>(ResolutionToolKind::SourceResolution, m_logger);
        case Tools::PlaneResolution:
            return winrt::make<ResolutionTool>(ResolutionToolKind::PlaneResolution, m_logger);
        case Tools::SourcePixelFormat:
            return winrt::make<PixelFormatTool>(PixelFormatToolKind::SourcePixelFormat, m_logger);
        case Tools::SurfacePixelFormat:
            return winrt::make<PixelFormatTool>(PixelFormatToolKind::PlanePixelFormat, m_logger);
        }

        // The caller has asked for a tool that is not exposed from this toolbox
        m_logger.LogAssert(Name() + L"::GetTool was called with invalid tool name: " + toolName);
        throw winrt::hresult_invalid_argument();
    }

    void Toolbox::SetConfigData(IJsonValue data)
    {
    }
}
