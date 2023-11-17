import "pch.h";
#include "PixelFormatTool.h"

import PredictionRenderer;

using namespace winrt::MicrosoftDisplayCaptureTools::Framework::Helpers;
namespace winrt
{
    using namespace MicrosoftDisplayCaptureTools::ConfigurationTools;
    using namespace MicrosoftDisplayCaptureTools::Display;
    using namespace MicrosoftDisplayCaptureTools::Framework;
    using namespace winrt::Windows::Graphics;
    using namespace winrt::Windows::Graphics::DirectX;
    using namespace winrt::Windows::Devices::Display::Core;
} // namespace winrt

namespace winrt::BasicDisplayConfiguration::implementation
{
    static const std::wstring DefaultConfiguration = L"R8G8B8A8UIntNormalized_NotInterlaced_NotStereo";
    struct Configuration
    {
        bool Interlaced;
        bool Stereo;
        DirectXPixelFormat SourceFormat;
        uint8_t BitsPerPixel;
    };

    std::map<std::wstring, Configuration> ConfigurationMap
    {
        {L"R8G8B8A8UIntNormalized_NotInterlaced_NotStereo", {false, false, DirectXPixelFormat::R8G8B8A8UIntNormalized, 24}}
    };

    PixelFormatTool::PixelFormatTool(PixelFormatToolKind kind) :
        m_kind(kind),
        m_currentConfig(DefaultConfiguration)
    {
    }

    hstring PixelFormatTool::Name()
    {
        switch (m_kind)
        {
        case PixelFormatToolKind::SourcePixelFormat:
            return L"SourcePixelFormat";
        case PixelFormatToolKind::PlanePixelFormat:
            return L"PlanePixelFormat";
        }
        return L"PixelFormat";
    }

    ConfigurationToolCategory PixelFormatTool::Category()
    {
        return ConfigurationToolCategory::DisplaySetup;
    }

    IConfigurationToolRequirements PixelFormatTool::Requirements()
    {
        return nullptr;
    }

    com_array<hstring> PixelFormatTool::GetSupportedConfigurations()
    {
        std::vector<hstring> configs;
        for (auto&& config : ConfigurationMap)
        {
            hstring configName(config.first);
            configs.push_back(configName);
        }

        return com_array<hstring>(configs);
    }

    hstring PixelFormatTool::GetDefaultConfiguration()
    {
        hstring defaultConfig(DefaultConfiguration);
        return defaultConfig;
    }

    hstring PixelFormatTool::GetConfiguration()
    {
        hstring currentConfig(m_currentConfig);
        return currentConfig;
    }

    void PixelFormatTool::SetConfiguration(hstring configuration)
    {
        if (ConfigurationMap.find(configuration.c_str()) == ConfigurationMap.end())
        {
            // An invalid configuration was asked for
            Logger().LogError(L"An invalid configuration was requested: " + configuration);

            throw winrt::hresult_invalid_argument();
        }

        m_currentConfig = configuration.c_str();
    }

    void PixelFormatTool::ApplyToOutput(IDisplayOutput displayOutput)
    {
        m_displaySetupEventToken = displayOutput.DisplaySetupCallback([this](const auto&, IDisplaySetupToolArgs args)
        {
            if (m_kind == PixelFormatToolKind::SourcePixelFormat)
            {
                auto interlaced = args.Mode().IsInterlaced();
                auto stereo = args.Mode().IsStereo();
                auto sourceFormat = args.Mode().SourcePixelFormat();

                auto& configValues = ConfigurationMap[m_currentConfig];
                const bool compatible = interlaced == configValues.Interlaced && stereo == configValues.Stereo &&
                                        sourceFormat == configValues.SourceFormat;
                args.IsModeCompatible(compatible);
            }
        });

        Logger().LogNote(L"Registering " + Name() + L": " + m_currentConfig + L" to be applied.");
    }

    void PixelFormatTool::ApplyToPrediction(IPrediction displayPrediction)
    {
        m_drawPredictionEventToken = displayPrediction.DisplaySetupCallback([this](const auto&, IPredictionData predictionData)
        {
            auto prediction = predictionData.as<PredictionRenderer::PredictionData>();

            for (auto& frame : prediction->Frames())
            {
                frame.WireFormat = winrt::DisplayWireFormat(
                    winrt::DisplayWireFormatPixelEncoding::Rgb444,
                    24, // bits per pixel
                    winrt::DisplayWireFormatColorSpace::BT709,
                    winrt::DisplayWireFormatEotf::Sdr,
                    winrt::DisplayWireFormatHdrMetadata::None);
            }
        });
    }
} // namespace winrt::BasicDisplayConfiguration::implementation