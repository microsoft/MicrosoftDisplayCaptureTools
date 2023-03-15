#include "pch.h"
#include "PixelFormatTool.h"

namespace winrt 
{
    using namespace MicrosoftDisplayCaptureTools::ConfigurationTools;
    using namespace MicrosoftDisplayCaptureTools::Display;
    using namespace MicrosoftDisplayCaptureTools::Framework;
    using namespace winrt::Windows::Graphics;
    using namespace winrt::Windows::Graphics::DirectX;
} // namespace winrt

namespace winrt::BasicDisplayConfiguration::implementation 
{
    static const std::wstring DefaultConfiguration = L"R8G8B8A8UIntNormalized_NotInterlaced";
    struct Configuration
    {
        bool Interlaced;
        DirectXPixelFormat SourceFormat;
        uint8_t BitsPerPixel;
    };

    std::map<std::wstring, Configuration> ConfigurationMap
    {
        {L"R8G8B8A8UIntNormalized_NotInterlaced", {false, DirectXPixelFormat::R8G8B8A8UIntNormalized, 32}}
    };

    PixelFormatTool::PixelFormatTool(winrt::ILogger const& logger) :
        m_currentConfig(DefaultConfiguration),
        m_logger(logger)
    {
    }

    hstring PixelFormatTool::Name()
    {
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
            m_logger.LogError(L"An invalid configuration was requested: " + configuration);

            throw winrt::hresult_invalid_argument();
        }

        m_currentConfig = configuration.c_str();
    }

    void PixelFormatTool::ApplyToOutput(IDisplayOutput displayOutput)
    {
        m_displaySetupEventToken = displayOutput.DisplaySetupCallback([this](const auto&, IDisplaySetupToolArgs args)
        {
            auto interlaced = args.Mode().IsInterlaced();
            auto sourceFormat = args.Mode().SourcePixelFormat();

            auto& configValues = ConfigurationMap[m_currentConfig];
            args.IsModeCompatible(interlaced == configValues.Interlaced && sourceFormat == configValues.SourceFormat);
        });

        m_logger.LogNote(L"Registering " + Name() + L": " + m_currentConfig + L" to be applied.");
    }

    void PixelFormatTool::ApplyToPrediction(IDisplayPrediction displayPrediction)
    {
        m_drawPredictionEventToken = displayPrediction.DisplaySetupCallback([this](const auto&, IDisplayPredictionData predictionData) 
        {
            auto desc = predictionData.FrameData().FormatDescription();
            desc.BitsPerPixel = ConfigurationMap[m_currentConfig].BitsPerPixel;
            desc.PixelFormat = ConfigurationMap[m_currentConfig].SourceFormat;
            predictionData.FrameData().FormatDescription(desc);
        });
    }
} // namespace winrt::BasicDisplayConfiguration::implementation