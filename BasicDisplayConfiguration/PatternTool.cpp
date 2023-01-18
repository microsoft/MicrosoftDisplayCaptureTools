#include "pch.h"
#include "PatternTool.h"

#include "winrt\Microsoft.Graphics.Canvas.h"
#include "winrt\Windows.UI.h"

namespace winrt
{
	using namespace MicrosoftDisplayCaptureTools::ConfigurationTools;
	using namespace MicrosoftDisplayCaptureTools::Display;
	using namespace MicrosoftDisplayCaptureTools::Framework;
	using namespace Windows::Storage::Streams;
	using namespace Windows::Graphics::Imaging;
	using namespace Microsoft::Graphics::Canvas;
	using namespace Windows::UI;
}


namespace winrt::BasicDisplayConfiguration::implementation
{

	std::map<PatternToolConfigurations, winrt::hstring> ConfigurationMap
	{
		{ PatternToolConfigurations::White, L"White" },
		{ PatternToolConfigurations::Red,   L"Red"   },
		{ PatternToolConfigurations::Green, L"Green" },
		{ PatternToolConfigurations::Blue,  L"Blue"  },
		{ PatternToolConfigurations::Gray,  L"Gray"  }
	};

	std::map<Windows::Graphics::DirectX::DirectXPixelFormat, uint32_t> SupportedFormatsWithSizePerPixel
	{
        { Windows::Graphics::DirectX::DirectXPixelFormat::R8G8B8A8UIntNormalized, 4 }
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
		return ConfigurationToolCategory::RenderSetup;
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

		if (SupportedFormatsWithSizePerPixel.find(planeProperties.Format()) == SupportedFormatsWithSizePerPixel.end())
		{
            m_logger.LogError(L"PatternTool does not support the plane pixel format.");
            return;
		}

		auto canvasDevice = CanvasDevice::GetSharedDevice();
        auto patternTarget = CanvasRenderTarget(
			canvasDevice,
			(float)displayProperties.Resolution().Width,
			(float)displayProperties.Resolution().Height,
			96,
			planeProperties.Format(),
			CanvasAlphaMode::Ignore);

        {
            auto drawingSession = patternTarget.CreateDrawingSession();
            Color checkerColor;

            switch (m_currentConfig)
            {
            case PatternToolConfigurations::White:
                checkerColor = Colors::White();
                break;
            case PatternToolConfigurations::Red:
                checkerColor = Colors::Red();
                break;
            case PatternToolConfigurations::Green:
                checkerColor = Colors::Green();
                break;
            case PatternToolConfigurations::Blue:
                checkerColor = Colors::Blue();
                break;
            case PatternToolConfigurations::Gray:
                checkerColor.R = 128;
                checkerColor.G = 128;
                checkerColor.B = 128;
                checkerColor.A = 255;
                break;
            }

            drawingSession.Clear(Colors::Black());

			bool indent = false;
			for (float x = 0; x < displayProperties.Resolution().Width; x += PatternToolSquareSize)
			{
                for (float y = indent ? (float)PatternToolSquareSize : 0.f; y < displayProperties.Resolution().Height; y += 2 * PatternToolSquareSize)
				{
                    drawingSession.FillRectangle(x, y, PatternToolSquareSize, PatternToolSquareSize, checkerColor);
				}

				indent = !indent;
			}

			drawingSession.Close();
        }

        auto bytesPerPixel = SupportedFormatsWithSizePerPixel[planeProperties.Format()];
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