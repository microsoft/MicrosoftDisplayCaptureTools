#include "pch.h"
#include "PatternTool.h"

#include "winrt\Microsoft.Graphics.Canvas.h"
#include "winrt\Windows.UI.h"

#include "..\Shared\Inc\DisplayToolComInterop.h"

#define PATTERN_SQUARE_SIZE 50.f


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

	void PatternTool::ApplyToOutput(IDisplayOutput displayOutput)
    {
        m_drawCallbackToken = displayOutput.RenderSetupCallback([this](const auto&, IRenderSetupToolArgs args)
        {
            m_logger.LogNote(L"Using " + Name() + L": " + ConfigurationMap[m_currentConfig]);

            auto sourceModeFormat = args.Properties().ActiveMode().SourcePixelFormat();
            auto sourceModeResolution = args.Properties().ActiveMode().SourceResolution();

            if (SupportedFormatsWithSizePerPixel.find(sourceModeFormat) == SupportedFormatsWithSizePerPixel.end())
            {
                m_logger.LogError(L"PatternTool does not support the plane pixel format.");
                return;
            }

            auto planeProperties = args.Properties().GetPlaneProperties()[0];

            auto dxgiSurface = winrt::MicrosoftDisplayCaptureTools::Libraries::GetMapEntry<IDXGISurface>(planeProperties, L"D3DSurface");

            winrt::com_ptr<ID2D1Factory> d2dFactory;
            winrt::check_hresult(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, d2dFactory.put()));
            winrt::com_ptr<ID2D1RenderTarget> d2dTarget;
            D2D1_RENDER_TARGET_PROPERTIES d2dRtProperties;
            d2dRtProperties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;
            d2dRtProperties.pixelFormat.format = DXGI_FORMAT_UNKNOWN;
            d2dRtProperties.type = D2D1_RENDER_TARGET_TYPE_DEFAULT;
            d2dRtProperties.minLevel = D2D1_FEATURE_LEVEL_10;
            d2dRtProperties.dpiX = 96.f;
            d2dRtProperties.dpiY = 96.f;
            d2dRtProperties.usage = D2D1_RENDER_TARGET_USAGE_NONE;

            winrt::check_hresult(d2dFactory->CreateDxgiSurfaceRenderTarget(dxgiSurface, d2dRtProperties, d2dTarget.put()));

            D2D1_COLOR_F checkerColor;
            switch (m_currentConfig)
            {
            case PatternToolConfigurations::Green:
                checkerColor = D2D1::ColorF(D2D1::ColorF::Green, 1.0f);
                break;
            case PatternToolConfigurations::White:
                checkerColor = D2D1::ColorF(D2D1::ColorF::White, 1.0f);
                break;
            case PatternToolConfigurations::Red:
                checkerColor = D2D1::ColorF(D2D1::ColorF::Red, 1.0f);
                break;
            case PatternToolConfigurations::Blue:
                checkerColor = D2D1::ColorF(D2D1::ColorF::Blue, 1.0f);
                break;
            case PatternToolConfigurations::Gray:
                checkerColor = D2D1::ColorF(D2D1::ColorF::Gray, 1.0f);
                break;
            }

            winrt::com_ptr<ID2D1SolidColorBrush> checkerBrush;
            winrt::check_hresult(d2dTarget->CreateSolidColorBrush(checkerColor, checkerBrush.put()));

			d2dTarget->BeginDraw();
            d2dTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black, 1.0f));

            bool indent = false;
            for (float x = 0; x < sourceModeResolution.Width; x += PATTERN_SQUARE_SIZE)
            {
                for (float y = indent ? (float)PATTERN_SQUARE_SIZE : 0.f; y < sourceModeResolution.Height; y += 2 * PATTERN_SQUARE_SIZE)
                {
                    d2dTarget->FillRectangle(D2D1::RectF(x, y, x + PATTERN_SQUARE_SIZE, y + PATTERN_SQUARE_SIZE), checkerBrush.get());
                }

                indent = !indent;
            }

            winrt::check_hresult(d2dTarget->EndDraw());
        });
	}

    void PatternTool::ApplyToPrediction(IDisplayPrediction displayPrediction)
    {
        m_drawCallbackToken = displayPrediction.RenderSetupCallback([this](const auto&, IDisplayPredictionData predictionData) 
        {
            auto canvasDevice = CanvasDevice::GetSharedDevice();
            auto patternTarget = CanvasRenderTarget(
                canvasDevice,
                (float)predictionData.FrameData().Resolution().Width,
                (float)predictionData.FrameData().Resolution().Height,
                96,
                predictionData.FrameData().FormatDescription().PixelFormat,
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
                for (float x = 0; x < predictionData.FrameData().Resolution().Width; x += PATTERN_SQUARE_SIZE)
                {
                    for (float y = indent ? PATTERN_SQUARE_SIZE : 0.f; y < predictionData.FrameData().Resolution().Height;
                         y += 2 * PATTERN_SQUARE_SIZE)
                    {
                        drawingSession.FillRectangle(x, y, PATTERN_SQUARE_SIZE, PATTERN_SQUARE_SIZE, checkerColor);
                    }

                    indent = !indent;
                }

                drawingSession.Close();
            }

            auto bytesPerPixel = SupportedFormatsWithSizePerPixel[predictionData.FrameData().FormatDescription().PixelFormat];
            
            patternTarget.GetPixelBytes(
                predictionData.FrameData().Data(), 
                0, 
                0, 
                patternTarget.SizeInPixels().Width, 
                patternTarget.SizeInPixels().Height);
        });
        
    }
}