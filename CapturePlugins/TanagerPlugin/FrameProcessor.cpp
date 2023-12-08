#include "pch.h"
#include "FrameProcessor.h"

namespace PrecompiledShaders {
#include "ComputeShaders/Sampler_444_8bpc.h"
#include "ComputeShaders/Sampler_444_10bpc.h"
#include "ComputeShaders/Dequantizer.h"
#include "ComputeShaders/Ycbcr_ITUR_BT709.h"
#include "ComputeShaders/Linearize_ITUR_BT709.h"
#include "ComputeShaders/Color_ITUR_709.h"
#include "ComputeShaders/FrameSquaredDifferenceBucketSum.h"
} // namespace PrecompiledShaders

namespace winrt {
	using namespace Windows::Foundation;
	using namespace Windows::Foundation::Collections;
    using namespace Windows::Graphics;
	using namespace Windows::Graphics::Imaging;
	using namespace Windows::Graphics::DirectX;
	using namespace Windows::Graphics::DirectX::Direct3D11;
	using namespace Windows::Devices::Display;
	using namespace Windows::Devices::Display::Core;
	using namespace Windows::Storage;
	using namespace Windows::Storage::Streams;
	using namespace Windows::Data::Json;
	using namespace Windows::Devices::Enumeration;
	using namespace Windows::Devices::Usb;
	using namespace MicrosoftDisplayCaptureTools::CaptureCard;
	using namespace MicrosoftDisplayCaptureTools::ConfigurationTools;
	using namespace MicrosoftDisplayCaptureTools::Framework;
	using namespace MicrosoftDisplayCaptureTools::Display;
	using namespace MicrosoftDisplayCaptureTools::Framework::Helpers;
} // namespace winrt

namespace winrt::MicrosoftDisplayCaptureTools::TanagerPlugin::DataProcessing {

	FrameProcessor::FrameProcessor()
	{
        bool useWarp = !RuntimeSettings().GetSettingValueAsBool(L"RenderOnHardware");
        D3D_DRIVER_TYPE driverType = useWarp ? D3D_DRIVER_TYPE_WARP : D3D_DRIVER_TYPE_HARDWARE;

        UINT uCreationFlags = D3D11_CREATE_DEVICE_SINGLETHREADED;
        D3D_FEATURE_LEVEL flOut;

        static const D3D_FEATURE_LEVEL flvl[] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_11_1};
        winrt::check_hresult(D3D11CreateDevice(
            nullptr,
            driverType,
            nullptr,
            uCreationFlags,
            flvl,
            ARRAYSIZE(flvl),
            D3D11_SDK_VERSION,
            m_d3dDevice.put(),
            &flOut,
            m_d3dDeviceContext.put()));
    }

    winrt::com_ptr<ID3D11ComputeShader> FrameProcessor::GetShader(ComputeShaders shaderToLoad)
    {
        if (m_shaderCache.contains(shaderToLoad))
		{
            return m_shaderCache[shaderToLoad];
		}

        winrt::com_ptr<ID3D11ComputeShader> shader;

		switch (shaderToLoad)
		{
        case ComputeShaders::Skip:
            return nullptr;
        case ComputeShaders::Sampler_444_8bpc:
            winrt::check_hresult(m_d3dDevice->CreateComputeShader(
                PrecompiledShaders::Sampler_444_8bpc, sizeof(PrecompiledShaders::Sampler_444_8bpc), nullptr, shader.put()));
            break;
        case ComputeShaders::Sampler_444_10bpc:
            winrt::check_hresult(m_d3dDevice->CreateComputeShader(
                PrecompiledShaders::Sampler_444_10bpc, sizeof(PrecompiledShaders::Sampler_444_10bpc), nullptr, shader.put()));
            break;

		case ComputeShaders::Dequantizer:
            winrt::check_hresult(m_d3dDevice->CreateComputeShader(
                PrecompiledShaders::Dequantizer, sizeof(PrecompiledShaders::Dequantizer), nullptr, shader.put()));
            break;

        case ComputeShaders::Ycbcr_ITUR_BT709:
            winrt::check_hresult(m_d3dDevice->CreateComputeShader(
                PrecompiledShaders::Ycbcr_ITUR_BT709, sizeof(PrecompiledShaders::Ycbcr_ITUR_BT709), nullptr, shader.put()));
            break;

        case ComputeShaders::Linearize_ITUR_BT709:
            winrt::check_hresult(m_d3dDevice->CreateComputeShader(
                PrecompiledShaders::Linearize_ITUR_BT709, sizeof(PrecompiledShaders::Linearize_ITUR_BT709), nullptr, shader.put()));
            break;

        case ComputeShaders::Color_ITUR_709:
            winrt::check_hresult(m_d3dDevice->CreateComputeShader(
                PrecompiledShaders::Color_ITUR_709, sizeof(PrecompiledShaders::Color_ITUR_709), nullptr, shader.put()));
            break;
            
        case ComputeShaders::FrameSquaredDifferenceBucketSum:
            winrt::check_hresult(m_d3dDevice->CreateComputeShader(
                PrecompiledShaders::FrameSquaredDifferenceBucketSum,
                sizeof(PrecompiledShaders::FrameSquaredDifferenceBucketSum),
                nullptr,
                shader.put()));
            break;

        default:
			Logger().LogAssert(L"Attempted to load an unimplemented shader");
			throw winrt::hresult_not_implemented();
		}

		m_shaderCache[shaderToLoad] = shader;
		return shader;
    }

    void FrameProcessor::ClearShaderState()
    {
        m_d3dDeviceContext->CSSetShader(nullptr, nullptr, 0);
        ID3D11UnorderedAccessView* ppUAViewnullptr[1] = {nullptr};
        m_d3dDeviceContext->CSSetUnorderedAccessViews(0, 1, ppUAViewnullptr, nullptr);
        ID3D11ShaderResourceView* ppSRVnullptr[1] = {nullptr};
        m_d3dDeviceContext->CSSetShaderResources(0, 1, ppSRVnullptr);
        ID3D11Buffer* ppCBnullptr[1] = {nullptr};
        m_d3dDeviceContext->CSSetConstantBuffers(0, 1, ppCBnullptr);
    }

    template <typename PixelDataType>
    winrt::Windows::Storage::Streams::IBuffer FrameProcessor::GetBufferFromTexture(ID3D11Texture2D* texture)
    {
        winrt::com_ptr<ID3D11Texture2D> readBackTex;
        D3D11_TEXTURE2D_DESC desc = {};
        texture->GetDesc(&desc);
        desc.BindFlags = 0;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        desc.Usage = D3D11_USAGE_STAGING;
        desc.MiscFlags = 0;
        winrt::check_hresult(m_d3dDevice->CreateTexture2D(&desc, nullptr, readBackTex.put()));
        m_d3dDeviceContext->CopyResource(readBackTex.get(), texture);

        D3D11_MAPPED_SUBRESOURCE mappedResource;
        winrt::check_hresult(m_d3dDeviceContext->Map(readBackTex.get(), 0, D3D11_MAP_READ, 0, &mappedResource));

        winrt::Buffer outputBuffer(desc.Height * desc.Width * sizeof(PixelDataType));
        outputBuffer.Length(desc.Height * desc.Width * sizeof(PixelDataType));

        auto approxData = reinterpret_cast<PixelDataType*>(mappedResource.pData);
        memcpy_s(outputBuffer.data(), outputBuffer.Capacity(), approxData, outputBuffer.Capacity());
        m_d3dDeviceContext->Unmap(readBackTex.get(), 0);

        outputBuffer.Length(outputBuffer.Capacity());

        return outputBuffer;
    }

    ComputeShaders FrameProcessor::GetSamplerShader(
        IteIt68051Plugin::VideoTiming* timing, IteIt68051Plugin::AviInfoframe* aviInfoframe, IteIt68051Plugin::ColorInformation* colorInfo)
    {

        // temp hack to see if I can get this pipeline working 
        colorInfo->inputColorInfo.colorDepth = 10;


        switch (aviInfoframe->GetColorFormat())
        {
        case IteIt68051Plugin::AviColorFormat::RGB:
        case IteIt68051Plugin::AviColorFormat::YUV444:
            switch (colorInfo->inputColorInfo.colorDepth)
            {
			case 8:
				return ComputeShaders::Sampler_444_8bpc;
			case 10:
				return ComputeShaders::Sampler_444_10bpc;
			default:
				Logger().LogError(L"Unsupported bit depth, no Tanager sampler shader available.");
				throw winrt::hresult_not_implemented();
			}
        case IteIt68051Plugin::AviColorFormat::YUV422:
            Logger().LogError(L"Unsupported color format YUV422, no Tanager sampler shader available.");
            break;
        case IteIt68051Plugin::AviColorFormat::YUV420:
            Logger().LogError(L"Unsupported color format YUV422, no Tanager sampler shader available.");
            break;
        }

        throw winrt::hresult_not_implemented();
    }

    ComputeShaders FrameProcessor::GetDequantizerShader(
        IteIt68051Plugin::VideoTiming* timing, IteIt68051Plugin::AviInfoframe* aviInfoframe, IteIt68051Plugin::ColorInformation* colorInfo)
    {
        return ComputeShaders::Dequantizer;
    }

    ComputeShaders FrameProcessor::GetColorFormatShader(
        IteIt68051Plugin::VideoTiming* timing, IteIt68051Plugin::AviInfoframe* aviInfoframe, IteIt68051Plugin::ColorInformation* colorInfo)
    {
        if (aviInfoframe->GetColorFormat() == IteIt68051Plugin::AviColorFormat::RGB)
        {
            return ComputeShaders::Skip;
        }

        switch (aviInfoframe->GetColorimetry())
        {
        case IteIt68051Plugin::AviColorimetry::ITUR_BT709:
			return ComputeShaders::Ycbcr_ITUR_BT709;
        default:
            Logger().LogError(L"Unsupported colorimetry, no Tanager color format shader available.");
			throw winrt::hresult_not_implemented();
        }
    }

    ComputeShaders FrameProcessor::GetTransferFunctionShader(
        IteIt68051Plugin::VideoTiming* timing, IteIt68051Plugin::AviInfoframe* aviInfoframe, IteIt68051Plugin::ColorInformation* colorInfo)
    {
        switch (aviInfoframe->GetColorimetry())
        {
        case IteIt68051Plugin::AviColorimetry::ITUR_BT709:
        case IteIt68051Plugin::AviColorimetry::DefaultForVIC: // TODO: temporary
            return ComputeShaders::Linearize_ITUR_BT709;
        default:
            Logger().LogError(L"Unsupported colorimetry, no Tanager transfer function shader available.");
            throw winrt::hresult_not_implemented();
        }
    }

    ComputeShaders FrameProcessor::GetColorspaceShader(
        IteIt68051Plugin::VideoTiming* timing, IteIt68051Plugin::AviInfoframe* aviInfoframe, IteIt68051Plugin::ColorInformation* colorInfo)
    {
        switch (aviInfoframe->GetColorimetry())
        {
        case IteIt68051Plugin::AviColorimetry::ITUR_BT709:
        case IteIt68051Plugin::AviColorimetry::DefaultForVIC: // TODO: temporary
			return ComputeShaders::Color_ITUR_709;
		default:
			Logger().LogError(L"Unsupported colorimetry, no Tanager colorspace shader available.");
			throw winrt::hresult_not_implemented();
		}
    }

	winrt::IRawFrame FrameProcessor::ProcessDataToFrame(
		IteIt68051Plugin::VideoTiming* timing,
		IteIt68051Plugin::AviInfoframe* aviInfoframe,
		IteIt68051Plugin::ColorInformation* colorInfo,
		uint8_t* data,
		uint32_t size)
	{
		if (timing == nullptr || aviInfoframe == nullptr || colorInfo == nullptr || data == nullptr || size == 0)
		{
			Logger().LogError(L"Invalid arguments passed to ProcessDataToFrame.");
			throw winrt::hresult_invalid_argument();
		}

        // We don't want to be doing running multiple shader passes on the same device at the same time.
		auto lock = std::scoped_lock(m_d3dRenderingMutex);

        // Get the shaders we'll need for this frame, these will log errors and throw if the frame's data can't be processed.
        // They will return null if a stage isn't necessary (i.e. - no need to dequantize full range).
        auto sampler          = GetShader(GetSamplerShader(timing, aviInfoframe, colorInfo));
        auto dequantizer      = GetShader(GetDequantizerShader(timing, aviInfoframe, colorInfo));
        auto colorFormat      = GetShader(GetColorFormatShader(timing, aviInfoframe, colorInfo));
        auto transferFunction = GetShader(GetTransferFunctionShader(timing, aviInfoframe, colorInfo));
        auto colorspace       = GetShader(GetColorspaceShader(timing, aviInfoframe, colorInfo));

        // The buffer and SRV that will be used to hold the input data and 
        // the texture and UAV to hold the sampled data (16-bpc float 444)
        winrt::com_ptr<ID3D11Buffer> inputBuffer{nullptr};
        winrt::com_ptr<ID3D11ShaderResourceView> inputBufferView{nullptr};
        winrt::com_ptr<ID3D11Texture2D> sampledTexture{nullptr};
        winrt::com_ptr<ID3D11UnorderedAccessView> sampledTextureView{nullptr};
        {
            D3D11_BUFFER_DESC desc = {};
            desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
            desc.ByteWidth = size;
            desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
            desc.StructureByteStride = sizeof(uint64_t);
            D3D11_SUBRESOURCE_DATA inputData;
            inputData.pSysMem = data;
            winrt::check_hresult(m_d3dDevice->CreateBuffer(&desc, &inputData, inputBuffer.put()));

            // Create input buffer SRV
            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
            srvDesc.BufferEx.FirstElement = 0;
            srvDesc.Format = DXGI_FORMAT_UNKNOWN;
            srvDesc.BufferEx.NumElements = desc.ByteWidth / desc.StructureByteStride;

            winrt::check_hresult(m_d3dDevice->CreateShaderResourceView(inputBuffer.get(), &srvDesc, inputBufferView.put()));

            D3D11_TEXTURE2D_DESC textureDesc = {};
            textureDesc.Width = timing->hActive;
            textureDesc.Height = timing->vActive;
            textureDesc.MipLevels = 1;
            textureDesc.ArraySize = 1;
            textureDesc.Format = DXGI_FORMAT_R16G16B16A16_UINT;
            textureDesc.SampleDesc.Count = 1;
            textureDesc.SampleDesc.Quality = 0;
            textureDesc.Usage = D3D11_USAGE_DEFAULT;
            textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
            textureDesc.CPUAccessFlags = 0;
            textureDesc.MiscFlags = 0;
            winrt::check_hresult(m_d3dDevice->CreateTexture2D(&textureDesc, 0, sampledTexture.put()));

            // Create output buffer UAV
            D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
            uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
            uavDesc.Format = DXGI_FORMAT_R16G16B16A16_UINT;
            uavDesc.Texture2D.MipSlice = 0;
            winrt::check_hresult(m_d3dDevice->CreateUnorderedAccessView(sampledTexture.get(), &uavDesc, sampledTextureView.put()));

            // Run the shader
            {
                m_d3dDeviceContext->CSSetShader(sampler.get(), nullptr, 0);
                ID3D11ShaderResourceView* ppSRV[1] = {inputBufferView.get()};
                m_d3dDeviceContext->CSSetShaderResources(0, 1, ppSRV);
                ID3D11UnorderedAccessView* ppUAView[1] = {sampledTextureView.get()};
                m_d3dDeviceContext->CSSetUnorderedAccessViews(0, 1, ppUAView, nullptr);
                m_d3dDeviceContext->Dispatch(timing->hActive, timing->vActive, 1);
            }

            ClearShaderState();
        }

        // TEMP: read out the sampled texture data:
        {
            auto temp = GetBufferFromTexture<uint64_t>(sampledTexture.get());
            auto tempPtr = temp.data();
        }

        // The constant buffer for the dequantizer shader and the texture/UAV
        // to hold the dequantized data (16-bpc float 444)
        winrt::com_ptr<ID3D11Buffer> dequantizerConstantBuffer{nullptr};
        winrt::com_ptr<ID3D11Texture2D> dequantizedData{nullptr};
        winrt::com_ptr<ID3D11UnorderedAccessView> dequantizedDataView{nullptr};
        if (dequantizer != nullptr)
        {
            D3D11_TEXTURE2D_DESC textureDesc = {};
            textureDesc.Width = timing->hActive;
            textureDesc.Height = timing->vActive;
            textureDesc.MipLevels = 1;
            textureDesc.ArraySize = 1;
            textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
            textureDesc.SampleDesc.Count = 1;
            textureDesc.SampleDesc.Quality = 0;
            textureDesc.Usage = D3D11_USAGE_DEFAULT;
            textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
            textureDesc.CPUAccessFlags = 0;
            textureDesc.MiscFlags = 0;
            winrt::check_hresult(m_d3dDevice->CreateTexture2D(&textureDesc, 0, dequantizedData.put()));

            // Create output buffer UAV
            D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
            uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
            uavDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
            uavDesc.Texture2D.MipSlice = 0;
            winrt::check_hresult(m_d3dDevice->CreateUnorderedAccessView(dequantizedData.get(), &uavDesc, dequantizedDataView.put()));

            // The constant buffer definition for the dequantizer shader indicates how many levels there are for
            // each channel, what the peak value is, and what the minimum quantized value is.
            //
            // Peak = 2^N (where N is the bit depth)
            // Levels = how many levels has the output been quantized to (Generally 219*N^(N-8))
            // Min = what is the minimum value that the output has been quantized to (generally 16*N^(N-8))
            struct DequantizerConstantBuffer
            {
                uint32_t PeakForBitDepth;
                uint32_t A_min, A_levels;
                uint32_t B_min, B_levels;
                uint32_t C_min, C_levels;
                uint32_t pad; // included because CS constant buffers must be 16-byte aligned
            };

            auto ConstantBuffer = std::make_shared<DequantizerConstantBuffer>();

            uint32_t bitDepth = colorInfo->inputColorInfo.colorDepth;
            ConstantBuffer->PeakForBitDepth = (uint32_t)pow(2, bitDepth) - 1;
            
            if (aviInfoframe->GetPixelRange() == IteIt68051Plugin::AviPixelRange::Full)
            {
                ConstantBuffer->A_min = 0;
                ConstantBuffer->A_levels = ConstantBuffer->PeakForBitDepth;

                ConstantBuffer->B_min = 0;
                ConstantBuffer->B_levels = ConstantBuffer->PeakForBitDepth;

                ConstantBuffer->C_min = 0;
                ConstantBuffer->C_levels = ConstantBuffer->PeakForBitDepth;
            }
            else
            {
                ConstantBuffer->A_min = 16 * (uint32_t)pow(2, bitDepth - 8);
                ConstantBuffer->A_levels = (235 - 16) * (uint32_t)pow(2, bitDepth - 8);

                switch (aviInfoframe->GetColorFormat())
                {
                case IteIt68051Plugin::AviColorFormat::RGB:
                    ConstantBuffer->B_min = 16 * (uint32_t)pow(2, bitDepth - 8);
                    ConstantBuffer->B_levels = (235 - 16) * (uint32_t)pow(2, bitDepth - 8);
                    ConstantBuffer->C_min = 16 * (uint32_t)pow(2, bitDepth - 8);
                    ConstantBuffer->C_levels = (235 - 16) * (uint32_t)pow(2, bitDepth - 8);
                    break;
                default:
                    ConstantBuffer->B_min = 16 * (uint32_t)pow(2, bitDepth - 8);
                    ConstantBuffer->B_levels = (240 - 16) * (uint32_t)pow(2, bitDepth - 8);
                    ConstantBuffer->C_min = 16 * (uint32_t)pow(2, bitDepth - 8);
                    ConstantBuffer->C_levels = (240 - 16) * (uint32_t)pow(2, bitDepth - 8);
                }
            }

            D3D11_BUFFER_DESC desc = {};
            desc.Usage = D3D11_USAGE_DYNAMIC;
            desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            desc.ByteWidth = sizeof(DequantizerConstantBuffer);
            desc.MiscFlags = 0;
            desc.StructureByteStride = 0;

            D3D11_SUBRESOURCE_DATA InitData = {};
            InitData.pSysMem = ConstantBuffer.get();
            InitData.SysMemPitch = 0;
            InitData.SysMemSlicePitch = 0;
            winrt::check_hresult(m_d3dDevice->CreateBuffer(&desc, &InitData, dequantizerConstantBuffer.put()));

            // Run the shader
            {
                m_d3dDeviceContext->CSSetShader(dequantizer.get(), nullptr, 0);
                ID3D11UnorderedAccessView* ppUAView[2] = {sampledTextureView.get(), dequantizedDataView.get()};
                m_d3dDeviceContext->CSSetUnorderedAccessViews(0, 2, ppUAView, nullptr);
                ID3D11Buffer* ppCB[1] = {dequantizerConstantBuffer.get()};
                m_d3dDeviceContext->CSSetConstantBuffers(0, 1, ppCB);
                m_d3dDeviceContext->Dispatch(timing->hActive, timing->vActive, 1);
            }

            ClearShaderState();
        }
        else
        {
            // The quantization stage _can't_ be skipped, it's also where we convert the data to 16-bpc float 444.
            Logger().LogAssert(L"Attempted to skip the quantization stage.");
            throw winrt::hresult_error();
        }

        // TEMP: read out the dequantized texture data:
        {
            auto temp = GetBufferFromTexture<uint64_t>(dequantizedData.get());
            auto tempPtr = temp.data();
        }

        // The texture and UAV to hold the R'G'B' data (16-bpc float 444 R'G'B')
        winrt::com_ptr<ID3D11Texture2D> rgbPrime{nullptr};
        winrt::com_ptr<ID3D11UnorderedAccessView> rgbPrimeView{nullptr};
        if (colorFormat != nullptr)
        {
            D3D11_TEXTURE2D_DESC textureDesc = {};
            textureDesc.Width = timing->hActive;
            textureDesc.Height = timing->vActive;
            textureDesc.MipLevels = 1;
            textureDesc.ArraySize = 1;
            textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
            textureDesc.SampleDesc.Count = 1;
            textureDesc.SampleDesc.Quality = 0;
            textureDesc.Usage = D3D11_USAGE_DEFAULT;
            textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
            textureDesc.CPUAccessFlags = 0;
            textureDesc.MiscFlags = 0;
            winrt::check_hresult(m_d3dDevice->CreateTexture2D(&textureDesc, 0, rgbPrime.put()));

            // Create output buffer UAV
            D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
            uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
            uavDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
            uavDesc.Texture2D.MipSlice = 0;
            winrt::check_hresult(m_d3dDevice->CreateUnorderedAccessView(rgbPrime.get(), &uavDesc, rgbPrimeView.put()));

            // Run the shader
            {
                m_d3dDeviceContext->CSSetShader(colorFormat.get(), nullptr, 0);
                ID3D11UnorderedAccessView* ppUAView[2] = {dequantizedDataView.get(), rgbPrimeView.get()};
                m_d3dDeviceContext->CSSetUnorderedAccessViews(0, 2, ppUAView, nullptr);
                m_d3dDeviceContext->Dispatch(timing->hActive, timing->vActive, 1);
            }

            ClearShaderState();
        }
        else
        {
            // We skipped the color format stage, so just use the dequantized data as the R'G'B' data.
            rgbPrime = dequantizedData;
            rgbPrimeView = dequantizedDataView;
        }

        // TEMP: read out the R'G'B' texture data:
        {
            auto temp = GetBufferFromTexture<uint64_t>(rgbPrime.get());
            auto tempPtr = temp.data();
        }

        // The texture and UAV to hold the RGB data (16-bpc float 444 RGB)
        winrt::com_ptr<ID3D11Texture2D> rgbLinear{nullptr};
        winrt::com_ptr<ID3D11UnorderedAccessView> rgbLinearView{nullptr};
        if (transferFunction != nullptr)
        {
            D3D11_TEXTURE2D_DESC textureDesc = {};
            textureDesc.Width = timing->hActive;
            textureDesc.Height = timing->vActive;
            textureDesc.MipLevels = 1;
            textureDesc.ArraySize = 1;
            textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
            textureDesc.SampleDesc.Count = 1;
            textureDesc.SampleDesc.Quality = 0;
            textureDesc.Usage = D3D11_USAGE_DEFAULT;
            textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
            textureDesc.CPUAccessFlags = 0;
            textureDesc.MiscFlags = 0;
            winrt::check_hresult(m_d3dDevice->CreateTexture2D(&textureDesc, 0, rgbLinear.put()));

            // Create output buffer UAV
            D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
            uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
            uavDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
            uavDesc.Texture2D.MipSlice = 0;
            winrt::check_hresult(m_d3dDevice->CreateUnorderedAccessView(rgbLinear.get(), &uavDesc, rgbLinearView.put()));

            // Run the shader
            {
                m_d3dDeviceContext->CSSetShader(transferFunction.get(), nullptr, 0);
                ID3D11UnorderedAccessView* ppUAView[2] = {rgbPrimeView.get(), rgbLinearView.get()};
                m_d3dDeviceContext->CSSetUnorderedAccessViews(0, 2, ppUAView, nullptr);
                m_d3dDeviceContext->Dispatch(timing->hActive, timing->vActive, 1);
            }

            ClearShaderState();
        }
        else
        {
            // We skipped the transfer function stage, so just use the RGB' data as the RGB data.
            rgbLinear = rgbPrime;
            rgbLinearView = rgbPrimeView;
        }

        // TEMP: read out the RGB texture data:
        {
            auto temp = GetBufferFromTexture<uint64_t>(rgbLinear.get());
            auto tempPtr = temp.data();
        }

        // The texture and UAV to hold the scRGB data (16-bpc float 444 scRGB) and 
        // the texture and UAV to hold the sRGB data (8-bpc float 444 sRGB)
        winrt::com_ptr<ID3D11Texture2D> scRGB{nullptr};
        winrt::com_ptr<ID3D11UnorderedAccessView> scRGBView{nullptr};
        winrt::com_ptr<ID3D11Texture2D> sRGB{nullptr};
        winrt::com_ptr<ID3D11UnorderedAccessView> sRGBView{nullptr};
        if (colorspace != nullptr)
        {
            {
                D3D11_TEXTURE2D_DESC textureDesc = {};
                textureDesc.Width = timing->hActive;
                textureDesc.Height = timing->vActive;
                textureDesc.MipLevels = 1;
                textureDesc.ArraySize = 1;
                textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
                textureDesc.SampleDesc.Count = 1;
                textureDesc.SampleDesc.Quality = 0;
                textureDesc.Usage = D3D11_USAGE_DEFAULT;
                textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
                textureDesc.CPUAccessFlags = 0;
                textureDesc.MiscFlags = 0;
                winrt::check_hresult(m_d3dDevice->CreateTexture2D(&textureDesc, 0, scRGB.put()));

                // Create output buffer UAV
                D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
                uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
                uavDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
                uavDesc.Texture2D.MipSlice = 0;
                winrt::check_hresult(m_d3dDevice->CreateUnorderedAccessView(scRGB.get(), &uavDesc, scRGBView.put()));
            }
            {
                D3D11_TEXTURE2D_DESC textureDesc = {};
                textureDesc.Width = timing->hActive;
                textureDesc.Height = timing->vActive;
                textureDesc.MipLevels = 1;
                textureDesc.ArraySize = 1;
                textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UINT;
                textureDesc.SampleDesc.Count = 1;
                textureDesc.SampleDesc.Quality = 0;
                textureDesc.Usage = D3D11_USAGE_DEFAULT;
                textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
                textureDesc.CPUAccessFlags = 0;
                textureDesc.MiscFlags = 0;
                winrt::check_hresult(m_d3dDevice->CreateTexture2D(&textureDesc, 0, sRGB.put()));

                // Create output buffer UAV
                D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
                uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
                uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UINT;
                uavDesc.Texture2D.MipSlice = 0;
                winrt::check_hresult(m_d3dDevice->CreateUnorderedAccessView(sRGB.get(), &uavDesc, sRGBView.put()));
            }

            // Run the shader
            {
                m_d3dDeviceContext->CSSetShader(colorspace.get(), nullptr, 0);
                ID3D11UnorderedAccessView* ppUAView[3] = {rgbLinearView.get(), sRGBView.get(), scRGBView.get()};
                m_d3dDeviceContext->CSSetUnorderedAccessViews(0, 3, ppUAView, nullptr);
                m_d3dDeviceContext->Dispatch(timing->hActive, timing->vActive, 1);
            }

            ClearShaderState();
        }
        else
        {
            // We can't skip the colorspace stage, it's where we convert the RGB data to scRGB and sRGB.
            Logger().LogAssert(L"Attempted to skip the colorspace stage.");
            throw winrt::hresult_error();
        }

        // The SoftwareBitmap to transfer the sRGB data to
        winrt::SoftwareBitmap renderableApproximation{nullptr};
        {
            auto outputDataRgba8 = GetBufferFromTexture<uint32_t>(sRGB.get());

            renderableApproximation = winrt::SoftwareBitmap::CreateCopyFromBuffer(
                outputDataRgba8, winrt::BitmapPixelFormat::Rgba8, timing->hActive, timing->vActive);
        }

        auto scRGBBuffer = GetBufferFromTexture<uint64_t>(scRGB.get());

        {
            auto scRGBBufferPtr = scRGBBuffer.data();
        }

        return winrt::make<Frame>(winrt::SizeInt32{timing->hActive, timing->vActive}, scRGBBuffer, renderableApproximation);
    }

    double FrameProcessor::ComputePSNR(winrt::IRawFrame target, winrt::IRawFrame capture)
    {
        if (target == nullptr || capture == nullptr)
        {
            Logger().LogError(L"Invalid arguments passed to ComputePSNR.");
            throw winrt::hresult_invalid_argument();
        }

        auto lock = std::scoped_lock(m_d3dRenderingMutex);

		// TODO: implement the rest of this function
	}

    // Create a single frame capture object from the raw captured data.
    TanagerDisplayCapture::TanagerDisplayCapture(
        std::vector<byte> pixels,
        IteIt68051Plugin::VideoTiming* timing,
        IteIt68051Plugin::AviInfoframe* aviInfoframe,
        IteIt68051Plugin::ColorInformation* colorInfo)
    {
        if (timing == nullptr || aviInfoframe == nullptr || colorInfo == nullptr || pixels.empty())
        {
            Logger().LogError(L"Invalid arguments passed to TanagerDisplayCapture constructor");
            throw winrt::hresult_invalid_argument();
        }

        // Process the frame data
        {
            m_frames = winrt::single_threaded_vector<winrt::IRawFrame>();

            auto frame = FrameProcessor::GetInstance().ProcessDataToFrame(
                timing, aviInfoframe, colorInfo, pixels.data(), static_cast<uint32_t>(pixels.size()));

            m_frames.Append(frame);
        }

        // Set up the properties
        {
            // The properties are the same for all frames in the set
			m_properties = winrt::single_threaded_map<winrt::hstring, winrt::IInspectable>();

            // The properties for this specific _Tanager_ capture
            m_extendedProps = winrt::single_threaded_map<winrt::hstring, winrt::IInspectable>();
        }
        
    }

    bool TanagerDisplayCapture::CompareCaptureToPrediction(winrt::hstring name, winrt::MicrosoftDisplayCaptureTools::Framework::IRawFrameSet prediction)
    {
        if (prediction.Frames().Size() != m_frames.Size())
        {
            Logger().LogError(
                winrt::hstring(L"Prediction frame count did not match capture frame count. Predicted ") +
                std::to_wstring(prediction.Frames().Size()) + L", Captured " + std::to_wstring(m_frames.Size()));

            return false;
        }

        for (uint32_t index = 0; index < prediction.Frames().Size(); index++)
        {
            auto predictedFrame = prediction.Frames().GetAt(index);
            auto capturedFrame = m_frames.GetAt(index);

            {
                bool formatMismatch = false;

                // Compare the frame resolutions
                auto predictedFrameRes = predictedFrame.Resolution();
                auto capturedFrameRes = capturedFrame.Resolution();
                if (predictedFrameRes.Width != capturedFrameRes.Width || predictedFrameRes.Height != capturedFrameRes.Height)
                {
                    Logger().LogError(
                        winrt::hstring(L"Capture resolution did not match prediction. Captured=") +
                        std::to_wstring(capturedFrameRes.Width) + L"x" + std::to_wstring(capturedFrameRes.Height) + L", Predicted=" +
                        std::to_wstring(predictedFrameRes.Width) + L"x" + std::to_wstring(predictedFrameRes.Height));
                    formatMismatch = true;
                }

                // Compare the frame formats
                auto predictedFrameFormat = predictedFrame.DataFormat();
                auto capturedFrameFormat = capturedFrame.DataFormat();

                if (predictedFrameFormat.PixelEncoding() != capturedFrameFormat.PixelEncoding())
                {
                    Logger().LogError(winrt::hstring(L"Capture pixel format did not match prediction"));
                    formatMismatch = true;
                }

                if (predictedFrameFormat.ColorSpace() != capturedFrameFormat.ColorSpace())
                {
                    Logger().LogError(winrt::hstring(L"Capture color space did not match prediction"));
                    formatMismatch = true;
                }

                if (predictedFrameFormat.Eotf() != capturedFrameFormat.Eotf())
                {
                    Logger().LogError(winrt::hstring(L"Capture EOTF did not match prediction"));
                    formatMismatch = true;
                }

                if (predictedFrameFormat.HdrMetadata() != capturedFrameFormat.HdrMetadata())
                {
                    Logger().LogError(winrt::hstring(L"Capture HDR metadata format did not match prediction"));
                    formatMismatch = true;
                }

                if (predictedFrameFormat.BitsPerChannel() != capturedFrameFormat.BitsPerChannel())
                {
                    Logger().LogError(
                        winrt::hstring(L"Capture bits per channel did not match prediction. Captured=") +
                        std::to_wstring(capturedFrameFormat.BitsPerChannel()) + L", Predicted=" +
                        std::to_wstring(predictedFrameFormat.BitsPerChannel()));
                    formatMismatch = true;
                }

                if (formatMismatch)
                {
                    return false;
                }
            }

            auto psnr = FrameProcessor::GetInstance().ComputePSNR(predictedFrame, capturedFrame);

            if (psnr < 30.0)
            {
                Logger().LogError(
					winrt::hstring(L"PSNR for frame ") + std::to_wstring(index) + L" was " + std::to_wstring(psnr) +
					L", which is below the threshold of 30.0");

				return false;
			}
        }

        return true;
    }

    winrt::IRawFrameSet TanagerDisplayCapture::GetFrameData()
    {
        return this->get_strong().try_as<winrt::MicrosoftDisplayCaptureTools::Framework::IRawFrameSet>();
    }

    winrt::IVector<winrt::IRawFrame> TanagerDisplayCapture::Frames()
    {
        return m_frames;
    }

    winrt::IMapView<winrt::hstring, winrt::IInspectable> TanagerDisplayCapture::ExtendedProperties()
    {
        return m_extendedProps.GetView();
    }

    winrt::IMap<winrt::hstring, winrt::IInspectable> TanagerDisplayCapture::Properties()
    {
        return m_properties;
    }

    Frame::Frame(winrt::SizeInt32 const& resolution, winrt::IBuffer const& data, winrt::SoftwareBitmap const& bitmap) : 
        m_resolution(resolution),
		m_data(data), m_bitmap(bitmap)
    {
        m_properties = winrt::single_threaded_map<winrt::hstring, winrt::IInspectable>();
    }

    winrt::IMap<winrt::hstring, winrt::IInspectable> Frame::Properties()
    {
        return m_properties;
    }

    winrt::IBuffer Frame::Data()
    {
        return m_data;
    }

    winrt::DisplayWireFormat Frame::DataFormat()
    {
        return m_format;
    }

    winrt::SizeInt32 Frame::Resolution()
    {
        return m_resolution;
    }

    winrt::IAsyncOperation<winrt::SoftwareBitmap> Frame::GetRenderableApproximationAsync()
    {
        // In this implementation, this approximation is created as a side effect of rendering the prediction. So here the result
        // can just be returned.
        co_return m_bitmap;
    }

    winrt::hstring Frame::GetPixelInfo(uint32_t x, uint32_t y)
    {
        // TODO: implement this - the intent is that because the above returns only an approximation, this should return a
        // description in string form of what the original pixel values are for a given address.
        throw winrt::hresult_not_implemented();
    }
} // namespace winrt::MicrosoftDisplayCaptureTools::TanagerPlugin::DataProcessing