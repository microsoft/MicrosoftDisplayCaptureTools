#include "pch.h"
#include "ResolutionTool.h"

namespace winrt
{
	using namespace MicrosoftDisplayCaptureTools::ConfigurationTools;
	using namespace MicrosoftDisplayCaptureTools::Display;
	using namespace MicrosoftDisplayCaptureTools::Framework;
}

namespace winrt::BasicDisplayConfiguration::implementation
{
	std::map<ResolutionToolConfigurations, winrt::hstring> ConfigurationMap
	{
		{ ResolutionToolConfigurations::w1920h1080, L"1920x1080" },
		{ ResolutionToolConfigurations::w800h600, L"800x600" }
	};

	ResolutionTool::ResolutionTool(winrt::ILogger const& logger) : 
		m_currentConfig(sc_defaultConfig), 
		m_logger(logger)
	{
	}

	hstring ResolutionTool::Name()
	{
		return L"Resolution";
	}

	ConfigurationToolCategory ResolutionTool::Category()
	{
		return ConfigurationToolCategory::DisplaySetup;
	}

	IConfigurationToolRequirements ResolutionTool::Requirements()
	{
		return nullptr;
	}

	com_array<hstring> ResolutionTool::GetSupportedConfigurations()
	{
		std::vector<hstring> configs;
		for (auto config : ConfigurationMap)
		{
			configs.push_back(config.second);
		}

		return com_array<hstring>(configs);
	}

	hstring ResolutionTool::GetDefaultConfiguration()
	{
		return ConfigurationMap[sc_defaultConfig];
	}

    hstring ResolutionTool::GetConfiguration()
    {
        return ConfigurationMap[m_currentConfig];
	}

	void ResolutionTool::SetConfiguration(hstring configuration)
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

	void ResolutionTool::ApplyToOutput(IDisplayOutput displayOutput)
    {
        m_displaySetupEventToken = displayOutput.DisplaySetupCallback([this](const auto&, IDisplaySetupToolArgs args) 
		{
            auto sourceRes = args.Mode().SourceResolution();
            auto targetRes = args.Mode().TargetResolution();

			switch (m_currentConfig)
            {
            case ResolutionToolConfigurations::w1920h1080:
				args.IsModeCompatible(sourceRes.Height == 1080 && sourceRes.Width == 1920 && 
					                  targetRes.Height == 1080 && targetRes.Width == 1920);
                return;
            case ResolutionToolConfigurations::w800h600:
                args.IsModeCompatible(sourceRes.Height == 600 && sourceRes.Width == 800 &&
				                      targetRes.Height == 600 && targetRes.Width == 800);
                return;
            default:
                m_logger.LogError(Name() + L" was set to use an invalid configuration option.");
            }
        });

        m_logger.LogNote(L"Registering " + Name() + L": " + ConfigurationMap[m_currentConfig] + L" to be applied.");
	}

	void ResolutionTool::ApplyToPrediction(IDisplayPrediction displayPrediction)
    {
        m_drawCallbackToken = displayPrediction.DisplaySetupCallback([this](const auto&, IDisplayPredictionData predictionData) 
		{ 
			switch (m_currentConfig)
			{
			case ResolutionToolConfigurations::w1920h1080:
                predictionData.FrameData().Resolution({1920, 1080});
                break;
            case ResolutionToolConfigurations::w800h600:
                predictionData.FrameData().Resolution({800, 600});
				break;
			}

		//TODO: this doesn't belong here, just here as a PoC
            auto formatDesc = predictionData.FrameData().FormatDescription();
            formatDesc.BitsPerPixel = 32;
            formatDesc.Eotf = winrt::Windows::Devices::Display::Core::DisplayWireFormatEotf::Sdr;
            formatDesc.PixelEncoding = winrt::Windows::Devices::Display::Core::DisplayWireFormatPixelEncoding::Rgb444;
            formatDesc.PixelFormat = winrt::Windows::Graphics::DirectX::DirectXPixelFormat::R8G8B8A8UIntNormalized;
            formatDesc.Stride = 4 * predictionData.FrameData().Resolution().Width;

			predictionData.FrameData().FormatDescription(formatDesc);
		});
	}
}