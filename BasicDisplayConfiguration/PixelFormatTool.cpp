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
    std::map<PixelFormatToolConfigurations, winrt::hstring> ConfigurationMap
    {
    {PixelFormatToolConfigurations::R8G8B8A8UIntNormalized_Interlaced, L"R8G8B8A8UIntNormalized_Interlaced"}
    };

    PixelFormatTool::PixelFormatTool(winrt::ILogger const& logger) : m_currentConfig(sc_defaultConfig), m_logger(logger)
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
        for (auto config : ConfigurationMap)
        {
            configs.push_back(config.second);
        }

        return com_array<hstring>(configs);
    }

    hstring PixelFormatTool::GetDefaultConfiguration()
    {
        return ConfigurationMap[sc_defaultConfig];
    }

    hstring PixelFormatTool::GetConfiguration()
    {
        return ConfigurationMap[m_currentConfig];
    }

    void PixelFormatTool::SetConfiguration(hstring configuration)
    {
        for (auto config : ConfigurationMap)
        {
            if (config.second == configuration)
            {
                m_currentConfig = config.first;
                return;
            }
        }

        // An invalid configuration was asked for
        m_logger.LogError(L"An invalid configuration was requested: " + configuration);

        throw winrt::hresult_invalid_argument();
    }

    void PixelFormatTool::ApplyToOutput(IDisplayOutput displayOutput)
    {
        m_displaySetupEventToken = displayOutput.DisplaySetupCallback([this](const auto&, IDisplaySetupToolArgs args)
        {
            auto interlaced = args.Mode().IsInterlaced();
            auto sourceMode = args.Mode().SourcePixelFormat();

            switch (m_currentConfig)
            {
            case PixelFormatToolConfigurations::R8G8B8A8UIntNormalized_Interlaced:
                args.IsModeCompatible(!interlaced && sourceMode == DirectXPixelFormat::R8G8B8A8UIntNormalized);
                return;

            default:
                m_logger.LogError(Name() + L" was set to use an invalid configuration option.");
            }
        });

        m_logger.LogNote(L"Registering " + Name() + L": " + ConfigurationMap[m_currentConfig] + L" to be applied.");
    }

    void PixelFormatTool::ApplyToPrediction(IDisplayPrediction displayPrediction)
    {
        // This tool doesn't currently matter to the output
    }
} // namespace winrt::BasicDisplayConfiguration::implementation