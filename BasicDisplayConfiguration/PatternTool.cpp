#include "pch.h"
#include "PatternTool.h"

namespace winrt
{
	using namespace MicrosoftDisplayCaptureTools::ConfigurationTools;
	using namespace MicrosoftDisplayCaptureTools::Display;
	using namespace MicrosoftDisplayCaptureTools::Framework;
	using namespace Windows::Storage::Streams;
	using namespace Windows::Graphics::Imaging;
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

		// Create a D2D Factory so we can draw the base patterns
        winrt::com_ptr<ID2D1Factory7> d2dFactory;
        winrt::check_hresult(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, d2dFactory.put()));

		// Initialize D3D objects - for this we don't particularly care which GPU is used.
        winrt::com_ptr<IDXGIFactory6> dxgiFactory;
        dxgiFactory.capture(&CreateDXGIFactory2, 0);

        winrt::com_ptr<IDXGIAdapter> dxgiAdapter;
		winrt::check_hresult(dxgiFactory->EnumAdapters(0, dxgiAdapter.put()));

		winrt::com_ptr<ID3D11DeviceContext> d3dContext;
        D3D_FEATURE_LEVEL featureLevel;
        winrt::com_ptr<ID3D11Device> device;
        winrt::check_hresult(D3D11CreateDevice(
            dxgiAdapter.get(),
            D3D_DRIVER_TYPE_UNKNOWN,
            nullptr,
            0,
            nullptr,
            0,
            D3D11_SDK_VERSION,
            device.put(),
            &featureLevel,
            d3dContext.put()));
        auto d3dDevice = device.as<ID3D11Device5>();
        auto dxgiDevice = device.as<IDXGIDevice>();

		winrt::com_ptr<ID2D1Device6> d2dDevice;
        winrt::check_hresult(d2dFactory->CreateDevice(dxgiDevice.get(), d2dDevice.put()));

		winrt::com_ptr<ID2D1DeviceContext4> d2dDeviceContext;
        winrt::check_hresult(d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, d2dDeviceContext.put()));

		D2D1_BITMAP_PROPERTIES1 bitmapProperties;
        bitmapProperties.dpiX = 96.f;
        bitmapProperties.dpiY = 96.f;
        bitmapProperties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;
        bitmapProperties.pixelFormat.format = (DXGI_FORMAT)planeProperties.Format();
        bitmapProperties.colorContext = nullptr;;
        bitmapProperties.bitmapOptions |= D2D1_BITMAP_OPTIONS_TARGET;

		D2D1_SIZE_U bitmapSize;
        bitmapSize.width = planeProperties.Rect().Width;
        bitmapSize.height = planeProperties.Rect().Height;

		winrt::com_ptr<ID2D1Bitmap1> d2dBitmapTarget, d2dBitmapMappable;
        winrt::check_hresult(d2dDeviceContext->CreateBitmap(bitmapSize, nullptr, 0, bitmapProperties, d2dBitmapTarget.put()));

		// Create a second bitmap specifically for copying back to the CPU space.
		bitmapProperties.bitmapOptions = D2D1_BITMAP_OPTIONS_CPU_READ | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
        winrt::check_hresult(d2dDeviceContext->CreateBitmap(bitmapSize, nullptr, 0, bitmapProperties, d2dBitmapMappable.put()));

		d2dDeviceContext->SetTarget(d2dBitmapTarget.get());

		d2dDeviceContext->BeginDraw();
        
		switch (m_currentConfig)
		{
		case PatternToolConfigurations::Black:
            d2dDeviceContext->Clear({0.f, 0.f, 0.f});
			break;
		case PatternToolConfigurations::White:
            d2dDeviceContext->Clear({1.f, 1.f, 1.f});
            break;
        case PatternToolConfigurations::Red:
            d2dDeviceContext->Clear({1.f, 0.f, 0.f});
            break;
        case PatternToolConfigurations::Green:
            d2dDeviceContext->Clear({0.f, 1.f, 0.f});
            break;
        case PatternToolConfigurations::Blue:
            d2dDeviceContext->Clear({0.f, 0.f, 1.f});
			break;
		}

		if (FAILED(d2dDeviceContext->EndDraw()))
        {
            m_logger.LogError(L"Pattern Tool: Failed to call EndDraw.");
        }

		// Convert the drawn image to a softwarebitmap for interchangeability
		if (FAILED(d2dBitmapMappable->CopyFromBitmap(nullptr, d2dBitmapTarget.get(), nullptr)))
		{
            m_logger.LogError(L"Pattern Tool: Failed to call CopyFromBitmap.");
		}

		D2D1_MAPPED_RECT mappedRect;
        if (FAILED(d2dBitmapMappable->Map(D2D1_MAP_OPTIONS_READ, &mappedRect)))
        {
            m_logger.LogError(L"Pattern Tool: Failed to call Map.");
		}

		// There are a couple potential optimizations to avoid this second/third copy - namely using a D3D 
		// Surface as the target, or writing our own native IBuffer implementation.
		winrt::array_view<byte> bitmapBytes(mappedRect.bits, mappedRect.pitch * bitmapSize.height);
        auto dataWriter = DataWriter();
        dataWriter.WriteBytes(bitmapBytes);
        auto buffer = dataWriter.DetachBuffer();

		BitmapPixelFormat outputPixelFormat = (BitmapPixelFormat)bitmapProperties.pixelFormat.format;
        auto outputBitmap = SoftwareBitmap::CreateCopyFromBuffer(buffer, outputPixelFormat, bitmapSize.width, bitmapSize.height); 

		planeProperties.SourceBitmap(outputBitmap);
	}
}