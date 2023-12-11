import "pch.h";
import PredictionRenderer;
//import RenderingUtils;

#include "BasePlanePattern.h"
#include "..\Shared\Inc\DisplayEngineInterop.h"

#define PATTERN_SQUARE_SIZE 50.f

using namespace winrt::MicrosoftDisplayCaptureTools::Framework::Helpers;
namespace winrt
{
	using namespace MicrosoftDisplayCaptureTools::ConfigurationTools;
	using namespace MicrosoftDisplayCaptureTools::Display;
	using namespace MicrosoftDisplayCaptureTools::Framework;
	using namespace Windows::Storage::Streams;
	using namespace Windows::Graphics::Imaging;
	using namespace Microsoft::Graphics::Canvas;
	using namespace Windows::UI;
    using namespace winrt::Windows::Graphics::DirectX;
}

namespace winrt::BasicDisplayConfiguration::implementation
{
    static const std::wstring DefaultConfiguration = L"Green";
    struct ConfigurationColor
    {
        float Red;
        float Green;
        float Blue;
    };

    // A map of the properties supported by this tool, mapping a name to R,G,B values
    std::map<std::wstring, ConfigurationColor> ConfigurationMap
    {
        {L"White",      {1.0f, 1.0f, 1.0f}},
        {L"Red",        {1.0f, 0.0f, 0.0f}},
        {L"Green",      {0.0f, 1.0f, 0.0f}},
        {L"Blue",       {0.0f, 0.0f, 1.0f}},
        {L"MediumGray", {0.5f, 0.5f, 0.5f}},
        {L"DarkGray",   {1.f / 3.f, 1.f / 3.f, 1.f / 3.f}},
        {L"LightGray",  {2.f / 3.f, 2.f / 3.f, 2.f / 3.f}},
    };

	std::map<Windows::Graphics::DirectX::DirectXPixelFormat, uint32_t> SupportedFormatsWithSizePerPixel
	{
        { Windows::Graphics::DirectX::DirectXPixelFormat::R8G8B8A8UIntNormalized, 4 }
	};

	BasePlanePattern::BasePlanePattern() :
        m_currentConfig(DefaultConfiguration)
	{
	}

	hstring BasePlanePattern::Name()
	{
		return L"Pattern";
	}

	ConfigurationToolCategory BasePlanePattern::Category()
	{
		return ConfigurationToolCategory::RenderSetup;
	}

	IConfigurationToolRequirements BasePlanePattern::Requirements()
	{
		return nullptr;
	}

	com_array<hstring> BasePlanePattern::GetSupportedConfigurations()
	{
		std::vector<hstring> configs;
		for (auto&& config : ConfigurationMap)
		{
            hstring configName(config.first);
            configs.push_back(configName);
		}

		return com_array<hstring>(configs);
	}

	hstring BasePlanePattern::GetDefaultConfiguration()
    {
        hstring defaultConfig(DefaultConfiguration);
        return defaultConfig;
	}

	hstring BasePlanePattern::GetConfiguration()
    {
        hstring currentConfig(m_currentConfig);
        return currentConfig;
	}

	void BasePlanePattern::SetConfiguration(hstring configuration)
	{
        if (ConfigurationMap.find(configuration.c_str()) == ConfigurationMap.end())
        {
            // An invalid configuration was asked for
            Logger().LogError(L"An invalid configuration was requested: " + configuration);

            throw winrt::hresult_invalid_argument();
        }

        m_currentConfig = configuration.c_str();
	}

	void BasePlanePattern::ApplyToOutput(IDisplayOutput displayOutput)
    {
        m_drawOutputEventToken = displayOutput.RenderSetupCallback([this](const auto&, IRenderSetupToolArgs args) {
            Logger().LogNote(L"Using " + Name() + L": " + m_currentConfig);

            auto sourceModeFormat = args.Properties().ActiveMode().SourcePixelFormat();
            auto sourceModeResolution = args.Properties().ActiveMode().SourceResolution();

            if (SupportedFormatsWithSizePerPixel.find(sourceModeFormat) == SupportedFormatsWithSizePerPixel.end())
            {
                Logger().LogError(L"BasePlanePattern does not support the plane pixel format.");
                throw winrt::hresult_invalid_argument();
                return;
            }

            auto planeProperties = args.Properties().GetPlaneProperties()[0];

            winrt::com_ptr<IDXGISurface> dxgiSurface;
            {
                winrt::com_ptr<ID3D11Texture2D> texture;
                auto planePropertiesInterop = planeProperties.try_as<IDisplayEnginePlanePropertiesInterop>();

                if (!planePropertiesInterop)
                {
                    Logger().LogError(Name() + L": this tool requires the DisplayEngine to expose IDisplayEnginePlanePropertiesInterop.");
                    throw winrt::hresult_class_not_available();
                }

                winrt::check_hresult(planePropertiesInterop->GetPlaneTexture(texture.put()));
                dxgiSurface = texture.as<IDXGISurface>();
            }

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

            winrt::check_hresult(d2dFactory->CreateDxgiSurfaceRenderTarget(dxgiSurface.get(), d2dRtProperties, d2dTarget.put()));

            auto& configColor = ConfigurationMap[m_currentConfig];
            D2D1_COLOR_F checkerColor = D2D1::ColorF(
                configColor.Red,
                configColor.Green,
                configColor.Blue
            );

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

    void BasePlanePattern::RenderPatternToPlane(const CanvasDrawingSession& drawingSession, float width, float height)
    {
        drawingSession.Clear(Colors::Black());

        auto& configColor = ConfigurationMap[m_currentConfig];
        Color checkerColor;
        checkerColor.A = 255;
        checkerColor.R = static_cast<uint8_t>(255 * configColor.Red);
        checkerColor.G = static_cast<uint8_t>(255 * configColor.Green);
        checkerColor.B = static_cast<uint8_t>(255 * configColor.Blue);
        auto checkerColorBrush = winrt::Brushes::CanvasSolidColorBrush::CreateHdr(drawingSession, {configColor.Red, configColor.Green, configColor.Blue, 1.0f});

        bool indent = false;
        for (float x = 0; x < width; x += PATTERN_SQUARE_SIZE)
        {
            for (float y = indent ? PATTERN_SQUARE_SIZE : 0.f; y < height; y += 2 * PATTERN_SQUARE_SIZE)
            {
                drawingSession.FillRectangle(x, y, PATTERN_SQUARE_SIZE, PATTERN_SQUARE_SIZE, checkerColorBrush);
            }

            indent = !indent;
        }
    }

    static DirectXPixelFormat PixelFormatFromPlaneInformation(const PredictionRenderer::PlaneInformation& plane)
    {
        if (plane.ColorType == PredictionRenderer::PlaneColorType::RGB)
        {
            switch (plane.ColorSpace)
            {
            case DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709:
                return DirectXPixelFormat::R8G8B8A8UIntNormalizedSrgb;
            }
        }

        // TODO: this needs to support more formats - should I drop into D2D/D3D directly to handle YUV formats?
        return DirectXPixelFormat::Unknown;
    }

    void BasePlanePattern::ApplyToPrediction(IPrediction displayPrediction)
    {
        m_drawPredictionEventToken = displayPrediction.RenderSetupCallback([this](const auto&, IPredictionData predictionData)
        {
            // IPredictionData is the generic interface, this tool requires the underlying type to be PredictionData from this binary
            auto prediction = predictionData.as<PredictionRenderer::PredictionData>();

            for (auto& frame : prediction->Frames())
            {
                int basePlanesPerFrame = 0;
                for (auto& plane : frame.Planes)
                {
                    if (plane.Type == PredictionRenderer::PlaneType::BasePlane)
                    {
                        basePlanesPerFrame++;

                        if (basePlanesPerFrame > 1)
                        {
                            Logger().LogError(Name() + L": More than one plane has been designated the 'base' plane per frame!");
                            throw winrt::hresult_invalid_argument();
                        }

                        auto pixelFormat = PixelFormatFromPlaneInformation(plane);
                        if (pixelFormat == DirectXPixelFormat::Unknown)
                        {
                            Logger().LogError(Name() + L": The plane format configuration (color type and color space) is not supported by this tool!");
                            throw winrt::hresult_invalid_argument();
                        }

                        auto canvasDevice = prediction->Device();
                        auto patternTarget = CanvasRenderTarget(
                            canvasDevice,
                            frame.SourceModeSize.Width,
                            frame.SourceModeSize.Height,
                            96, pixelFormat,
                            CanvasAlphaMode::Premultiplied);

                        {
                            auto drawingSession = patternTarget.CreateDrawingSession();

                            RenderPatternToPlane(
                                drawingSession,
                                frame.SourceModeSize.Width,
                                frame.SourceModeSize.Height);

                            drawingSession.Flush();
                            drawingSession.Close();
                        }

                        // Transfer the rendered plane to the prediction plane surface - which will be composed.
                        plane.Surface = patternTarget.as<winrt::Direct3D11::IDirect3DSurface>();
                    }

                }
            }
        });
    }
}