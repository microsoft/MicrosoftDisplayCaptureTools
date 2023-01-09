#include "pch.h"
#include "PatternTool.h"

#include "winrt\Microsoft.Graphics.Canvas.h"

namespace winrt
{
	using namespace MicrosoftDisplayCaptureTools::ConfigurationTools;
	using namespace MicrosoftDisplayCaptureTools::Display;
	using namespace MicrosoftDisplayCaptureTools::Framework;
	using namespace Windows::Storage::Streams;
	using namespace Windows::Graphics::Imaging;
	using namespace Microsoft::Graphics::Canvas;
}


namespace winrt::BasicDisplayConfiguration::implementation
{

	std::map<PatternToolConfigurations, winrt::hstring> ConfigurationMap
	{
		{ PatternToolConfigurations::Black, L"Black" },
		{ PatternToolConfigurations::White, L"White" },
		{ PatternToolConfigurations::Red,   L"Red"   },
		{ PatternToolConfigurations::Green, L"Green" },
		{ PatternToolConfigurations::Blue,  L"Blue"  }
	};

	PatternTool::PatternTool(winrt::ILogger const& logger) :
		m_currentConfig(sc_defaultConfig),
		m_logger(logger)
	{
	}

	hstring PatternTool::Name()
	{
		return L"Pattern";
	}

	ConfigurationToolCategory PatternTool::Category()
	{
		return ConfigurationToolCategory::Render;
	}

	IConfigurationToolRequirements PatternTool::Requirements()
	{
		return nullptr;
	}

	com_array<hstring> PatternTool::GetSupportedConfigurations()
	{
		std::vector<hstring> configs;
		for (auto config : ConfigurationMap)
		{
			configs.push_back(config.second);
		}

		return com_array<hstring>(configs);
	}

	hstring PatternTool::GetDefaultConfiguration()
	{
		return ConfigurationMap[sc_defaultConfig];
	}

	hstring PatternTool::GetConfiguration()
    {
        return ConfigurationMap[m_currentConfig];
	}

	void PatternTool::SetConfiguration(hstring configuration)
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

	void PatternTool::Apply(IDisplayOutput reference)
    {
        m_logger.LogNote(L"Using " + Name() + L": " + ConfigurationMap[m_currentConfig]);

		auto displayProperties = reference.GetProperties();
		auto planeProperties = displayProperties.GetPlaneProperties()[0];


		auto canvasDevice = CanvasDevice::GetSharedDevice();
        auto patternTarget = CanvasRenderTarget(
			canvasDevice,
			displayProperties.Resolution().Width,
			displayProperties.Resolution().Height,
			96,
			planeProperties.Format(),
			CanvasAlphaMode::Ignore);

        {
            auto drawingSession = patternTarget.CreateDrawingSession();
            winrt::Windows::Foundation::Numerics::float4 clearColor;

            switch (m_currentConfig)
            {
            case PatternToolConfigurations::Black:
                clearColor = {0.f, 0.f, 0.f, 1.f};
                break;
            case PatternToolConfigurations::White:
                clearColor = {1.f, 1.f, 1.f, 1.f};
                break;
            case PatternToolConfigurations::Red:
                clearColor = {1.f, 0.f, 0.f, 1.f};
                break;
            case PatternToolConfigurations::Green:
                clearColor = {0.f, 1.f, 0.f, 1.f};
                break;
            case PatternToolConfigurations::Blue:
                clearColor = {0.f, 0.f, 1.f, 1.f};
                break;
            }

            drawingSession.Clear(clearColor);
        }

		// TODO: update this bytes per pixel variable to match the actual format
		auto bytesPerPixel = 4;
        auto bufferSize = patternTarget.SizeInPixels().Height * patternTarget.SizeInPixels().Width * bytesPerPixel;
        auto pixelBuffer = Buffer(bufferSize);
        patternTarget.GetPixelBytes(pixelBuffer, 0, 0, patternTarget.SizeInPixels().Width, patternTarget.SizeInPixels().Height);

        auto resolution = Windows::Graphics::SizeInt32();
        resolution.Height = patternTarget.SizeInPixels().Height;
        resolution.Width = patternTarget.SizeInPixels().Width;

        auto planeImage = planeProperties.BaseImage();
        planeImage.Resolution(resolution);
        planeImage.Format(planeProperties.Format());
        planeImage.Pixels(pixelBuffer);
	}
}