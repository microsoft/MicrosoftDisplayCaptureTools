#include "pch.h"
#include "FrameProcessor.h"

namespace PrecompiledShaders {
#include "ComputeShaders/sRGB_8bpc_to_scRGB_16bpc.h"
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
    using namespace winrt::Windows::Media::Capture;
    using namespace MicrosoftDisplayCaptureTools::CaptureCard;
    using namespace MicrosoftDisplayCaptureTools::ConfigurationTools;
    using namespace MicrosoftDisplayCaptureTools::Framework;
    using namespace MicrosoftDisplayCaptureTools::Display;
    using namespace MicrosoftDisplayCaptureTools::Framework::Helpers;
} // namespace winrt

using namespace winrt::GenericCaptureCardPlugin::implementation;

namespace winrt::MicrosoftDisplayCaptureTools::GenericCaptureCardPlugin::DataProcessing {

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

        case ComputeShaders::sRGB_8bpc_to_scRGB_16bpc:
            winrt::check_hresult(m_d3dDevice->CreateComputeShader(
                PrecompiledShaders::sRGB_8bpc_to_scRGB_16bpc,
                sizeof(PrecompiledShaders::sRGB_8bpc_to_scRGB_16bpc),
                nullptr,
                shader.put()));
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

    winrt::IRawFrame FrameProcessor::ProcessDataToFrame(winrt::CapturedFrame frame)
    {
        if (!frame || !frame.SoftwareBitmap())
        {
			Logger().LogError(L"FrameProcessor::ProcessDataToFrame called with a frame that does not contain a SoftwareBitmap");
			throw winrt::hresult_invalid_argument();
		}

        // Retreive the already-encoded sRGB data from the frame
        auto capturedFrameBitmap = SoftwareBitmap::Convert(frame.SoftwareBitmap(), BitmapPixelFormat::Rgba8);

        // Create the frame object
        auto outputFrame = winrt::make_self<Frame>();

        // The data here is already in sRGB, so we only need to linearize it for our scRGB intermediate
        {
            auto buffer = capturedFrameBitmap.LockBuffer(BitmapBufferAccessMode::Read);
            auto reference = buffer.CreateReference();

            auto pixelData = reference.data();

            winrt::com_ptr<ID3D11Texture2D> inputTexture{nullptr};
            winrt::com_ptr<ID3D11ShaderResourceView> inputTextureView{nullptr};
            winrt::com_ptr<ID3D11Texture2D> outputTexture{nullptr};
            winrt::com_ptr<ID3D11UnorderedAccessView> outputTextureView{nullptr};
            {
                auto linearizeShader = GetShader(ComputeShaders::sRGB_8bpc_to_scRGB_16bpc);
                {
                    D3D11_TEXTURE2D_DESC textureDesc = {};
                    textureDesc.Width = capturedFrameBitmap.PixelWidth();
                    textureDesc.Height = capturedFrameBitmap.PixelHeight();
                    textureDesc.MipLevels = 1;
                    textureDesc.ArraySize = 1;
                    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UINT;
                    textureDesc.SampleDesc.Count = 1;
                    textureDesc.SampleDesc.Quality = 0;
                    textureDesc.Usage = D3D11_USAGE_DEFAULT;
                    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
                    textureDesc.CPUAccessFlags = 0;
                    textureDesc.MiscFlags = 0;

                    D3D11_SUBRESOURCE_DATA InitData = {};
                    InitData.pSysMem = pixelData;
                    InitData.SysMemPitch = capturedFrameBitmap.PixelWidth() * sizeof(uint32_t);
                    InitData.SysMemSlicePitch = 0;
                    winrt::check_hresult(m_d3dDevice->CreateTexture2D(&textureDesc, &InitData, inputTexture.put()));

                    // Create target texture SRV
                    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
                    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UINT;
                    srvDesc.Texture2D.MipLevels = 1;
                    winrt::check_hresult(m_d3dDevice->CreateShaderResourceView(inputTexture.get(), &srvDesc, inputTextureView.put()));
                }
                {
                    D3D11_TEXTURE2D_DESC textureDesc = {};
                    textureDesc.Width = capturedFrameBitmap.PixelWidth();
                    textureDesc.Height = capturedFrameBitmap.PixelHeight();
                    textureDesc.MipLevels = 1;
                    textureDesc.ArraySize = 1;
                    textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
                    textureDesc.SampleDesc.Count = 1;
                    textureDesc.SampleDesc.Quality = 0;
                    textureDesc.Usage = D3D11_USAGE_DEFAULT;
                    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
                    textureDesc.CPUAccessFlags = 0;
                    textureDesc.MiscFlags = 0;
                    winrt::check_hresult(m_d3dDevice->CreateTexture2D(&textureDesc, 0, outputTexture.put()));

                    // Create output buffer UAV
                    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
                    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
                    uavDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
                    uavDesc.Texture2D.MipSlice = 0;
                    winrt::check_hresult(m_d3dDevice->CreateUnorderedAccessView(outputTexture.get(), &uavDesc, outputTextureView.put()));
                }
                // Run the shader
                {
                    m_d3dDeviceContext->CSSetShader(linearizeShader.get(), nullptr, 0);
                    ID3D11ShaderResourceView* ppSRV[1] = {inputTextureView.get()};
                    m_d3dDeviceContext->CSSetShaderResources(0, 1, ppSRV);
                    ID3D11UnorderedAccessView* ppUAView[1] = {outputTextureView.get()};
                    m_d3dDeviceContext->CSSetUnorderedAccessViews(0, 1, ppUAView, nullptr);
                    m_d3dDeviceContext->Dispatch(capturedFrameBitmap.PixelWidth(), capturedFrameBitmap.PixelHeight(), 1);
                }

                ClearShaderState();
            }

            auto scRGBBuffer = GetBufferFromTexture<uint64_t>(outputTexture.get());

            outputFrame->SetBuffer(scRGBBuffer);
        }

        outputFrame->SetImageApproximation(capturedFrameBitmap);
        outputFrame->Resolution({capturedFrameBitmap.PixelWidth(), capturedFrameBitmap.PixelHeight()});

        return outputFrame.as<IRawFrame>();
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

    Frame::Frame() : m_data(nullptr), m_format(nullptr), m_resolution({0, 0}), m_bitmap(nullptr)
    {
        m_properties = winrt::single_threaded_map<winrt::hstring, winrt::IInspectable>();
    }

    winrt::IBuffer Frame::Data()
    {
        return m_data;
    }

    winrt::DisplayWireFormat Frame::DataFormat()
    {
        return m_format;
    }

    winrt::IMap<winrt::hstring, winrt::IInspectable> Frame::Properties()
    {
        return m_properties;
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
        UNREFERENCED_PARAMETER(x);
        UNREFERENCED_PARAMETER(y);

        throw winrt::hresult_not_implemented();
    }

    void Frame::SetBuffer(winrt::IBuffer buffer)
    {
        m_data = buffer;
    }

    void Frame::DataFormat(winrt::DisplayWireFormat const& description)
    {
        m_format = description;
    }

    void Frame::Resolution(winrt::SizeInt32 const& resolution)
    {
        m_resolution = resolution;
    }

    void Frame::SetImageApproximation(winrt::SoftwareBitmap bitmap)
    {
        m_bitmap = bitmap;
    }
    
    static winrt::IAsyncOperation<double> AddPixelSums(float* pixelSums, uint32_t startIndex, uint32_t processCount)
    {
        double sum = 0.0;
        for (uint32_t i = 0; i < processCount; i++)
        {
            sum += pixelSums[i + startIndex];
        }

        co_return sum;
    }

    double FrameProcessor::ComputePSNR(winrt::IRawFrame target, winrt::IRawFrame capture)
    {
        if (target == nullptr || capture == nullptr)
        {
            Logger().LogError(L"Invalid arguments passed to ComputePSNR.");
            throw winrt::hresult_invalid_argument();
        }

        auto lock = std::scoped_lock(m_d3dRenderingMutex);

        winrt::com_ptr<ID3D11Texture2D> targetTexture{nullptr};
        winrt::com_ptr<ID3D11ShaderResourceView> targetTextureView{nullptr};
        {
            D3D11_TEXTURE2D_DESC textureDesc = {};
            textureDesc.Width = target.Resolution().Width;
            textureDesc.Height = target.Resolution().Height;
            textureDesc.MipLevels = 1;
            textureDesc.ArraySize = 1;
            textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
            textureDesc.SampleDesc.Count = 1;
            textureDesc.SampleDesc.Quality = 0;
            textureDesc.Usage = D3D11_USAGE_DEFAULT;
            textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            textureDesc.CPUAccessFlags = 0;
            textureDesc.MiscFlags = 0;

            D3D11_SUBRESOURCE_DATA InitData = {};
            InitData.pSysMem = target.Data().data();
            InitData.SysMemPitch = target.Resolution().Width * sizeof(uint64_t);
            InitData.SysMemSlicePitch = 0;
            winrt::check_hresult(m_d3dDevice->CreateTexture2D(&textureDesc, &InitData, targetTexture.put()));

            // Create target texture SRV
            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
            srvDesc.Texture2D.MipLevels = 1;
            winrt::check_hresult(m_d3dDevice->CreateShaderResourceView(targetTexture.get(), &srvDesc, targetTextureView.put()));
        }

        winrt::com_ptr<ID3D11Texture2D> captureTexture{nullptr};
        winrt::com_ptr<ID3D11ShaderResourceView> captureTextureView{nullptr};
        {
            D3D11_TEXTURE2D_DESC textureDesc = {};
            textureDesc.Width = capture.Resolution().Width;
            textureDesc.Height = capture.Resolution().Height;
            textureDesc.MipLevels = 1;
            textureDesc.ArraySize = 1;
            textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
            textureDesc.SampleDesc.Count = 1;
            textureDesc.SampleDesc.Quality = 0;
            textureDesc.Usage = D3D11_USAGE_DEFAULT;
            textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            textureDesc.CPUAccessFlags = 0;
            textureDesc.MiscFlags = 0;

            D3D11_SUBRESOURCE_DATA InitData = {};
            InitData.pSysMem = capture.Data().data();
            InitData.SysMemPitch = capture.Resolution().Width * sizeof(uint64_t);
            InitData.SysMemSlicePitch = 0;
            winrt::check_hresult(m_d3dDevice->CreateTexture2D(&textureDesc, &InitData, captureTexture.put()));

            // Create target texture SRV
            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
            srvDesc.Texture2D.MipLevels = 1;
            winrt::check_hresult(m_d3dDevice->CreateShaderResourceView(captureTexture.get(), &srvDesc, captureTextureView.put()));
        }

        uint32_t numPixels = capture.Resolution().Width * capture.Resolution().Height;

        // The buffer and view that will hold the output of the summing shader
        winrt::com_ptr<ID3D11Texture2D> sumTexture{nullptr};
        winrt::com_ptr<ID3D11UnorderedAccessView> sumTextureView{nullptr};
        {
            D3D11_TEXTURE2D_DESC desc = {};
            desc.Format = DXGI_FORMAT_R32_FLOAT;
            desc.Width = capture.Resolution().Width;
            desc.Height = capture.Resolution().Height;
            desc.MipLevels = 1;
            desc.ArraySize = 1;
            desc.SampleDesc.Count = 1;
            desc.Usage = D3D11_USAGE_DEFAULT;
            desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
            winrt::check_hresult(m_d3dDevice->CreateTexture2D(&desc, 0, sumTexture.put()));

            // Create output buffer UAV
            D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
            uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
            uavDesc.Format = DXGI_FORMAT_R32_FLOAT;
            uavDesc.Texture1D.MipSlice = 0;
            winrt::check_hresult(m_d3dDevice->CreateUnorderedAccessView(sumTexture.get(), &uavDesc, sumTextureView.put()));
        }

        // Run the shader
        {
            auto sumShader = GetShader(ComputeShaders::FrameSquaredDifferenceBucketSum);
            m_d3dDeviceContext->CSSetShader(sumShader.get(), nullptr, 0);
            ID3D11ShaderResourceView* ppSRV[2] = {captureTextureView.get(), targetTextureView.get()};
            m_d3dDeviceContext->CSSetShaderResources(0, 2, ppSRV);
            ID3D11UnorderedAccessView* ppUAView[1] = {sumTextureView.get()};
            m_d3dDeviceContext->CSSetUnorderedAccessViews(0, 1, ppUAView, nullptr);
            m_d3dDeviceContext->Dispatch(target.Resolution().Width, target.Resolution().Height, 1);
        }

        ClearShaderState();

        // Map the sum array and sum the values
        {
            auto mappedData = GetBufferFromTexture<float>(sumTexture.get());

            auto sumTexturePtr = reinterpret_cast<float*>(mappedData.data());
            double sum = 0;

            constexpr uint32_t threadCount = 8;
            auto threads = std::vector<winrt::IAsyncOperation<double>>(threadCount);
            for (uint32_t i = 0; i < threadCount - 1; i++)
            {
                threads.push_back(AddPixelSums(sumTexturePtr, i * numPixels / threadCount, numPixels / threadCount));
            }

            // In the last thread account for any pixels that don't fall into a whole number of threadCount buckets
            threads.push_back(AddPixelSums(
                sumTexturePtr, (threadCount - 1) * numPixels / threadCount, numPixels / threadCount + numPixels % threadCount));

            for (auto& sumThread : threads)
            {
                sum += sumThread.get();
            }

            // Compute the PSNR
            double mse = sum / (static_cast<double>(numPixels) * 3);
            double psnr = 20.0 * log10((7.5 * 7.5) / mse); // 7.5 is the peak value for the scRGB format of our intermediates

            return psnr;
        }

        return 0;
    }

    DisplayCapture::DisplayCapture(winrt::CapturedFrame frame, winrt::IMap<winrt::hstring, winrt::IInspectable> extendedProps) :
        m_extendedProps(extendedProps)
    {
        // Ensure that we can actually read the provided frame
        if (!frame.CanRead())
        {
            Logger().LogError(L"Cannot read pixel data from frame.");
            throw winrt::hresult_invalid_argument();
        }

        {
            m_frames = winrt::single_threaded_vector<winrt::IRawFrame>();

            m_frames.Append(FrameProcessor::GetInstance().ProcessDataToFrame(frame));
        }

        // Set up the properties
        {
            // The properties are the same for all frames in the set
            m_properties = winrt::single_threaded_map<winrt::hstring, winrt::IInspectable>();

            // The properties for this specific _Tanager_ capture
            m_extendedProps = winrt::single_threaded_map<winrt::hstring, winrt::IInspectable>();
        }
    }

    bool DisplayCapture::CompareCaptureToPrediction(hstring name, MicrosoftDisplayCaptureTools::Framework::IRawFrameSet prediction)
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

            // Compare the frame resolutions
            auto predictedFrameRes = predictedFrame.Resolution();
            auto capturedFrameRes = capturedFrame.Resolution();
            if (predictedFrameRes.Width != capturedFrameRes.Width || predictedFrameRes.Height != capturedFrameRes.Height)
            {
                Logger().LogNote(
                    winrt::hstring(L"Capture resolution did not match prediction. Captured=") + std::to_wstring(capturedFrameRes.Width) +
                    L"x" + std::to_wstring(capturedFrameRes.Height) + L", Predicted=" + std::to_wstring(predictedFrameRes.Width) +
                    L"x" + std::to_wstring(predictedFrameRes.Height) + L" \nComparison will be to re-scaled data.");

                return false;
            }

            auto psnr = FrameProcessor::GetInstance().ComputePSNR(predictedFrame, capturedFrame);

            if (psnr < PsnrLimit)
            {
                Logger().LogError(
                    winrt::hstring(L"PSNR for frame ") + std::to_wstring(index) + L" was " + std::to_wstring(psnr) +
                    L", which is below the threshold of " + std::to_wstring(PsnrLimit));

                return false;
            }
            else
            {
                Logger().LogNote(
                    winrt::hstring(L"PSNR for frame ") + std::to_wstring(index) + L" was " + std::to_wstring(psnr) +
                    L", which is above the threshold of " + std::to_wstring(PsnrLimit));
            }
        }

        return true;
    }

    winrt::IRawFrameSet DisplayCapture::GetFrameData()
    {
        return this->get_strong().try_as<winrt::MicrosoftDisplayCaptureTools::Framework::IRawFrameSet>();
    }

    winrt::IVector<winrt::IRawFrame> DisplayCapture::Frames()
    {
        return m_frames;
    }

    winrt::IMapView<winrt::hstring, winrt::IInspectable> DisplayCapture::ExtendedProperties()
    {
        return m_extendedProps.GetView();
    }

    winrt::IMap<winrt::hstring, winrt::IInspectable> DisplayCapture::Properties()
    {
        return m_properties;
    }

} // namespace winrt::MicrosoftDisplayCaptureTools::GenericCaptureCardPlugin::DataProcessing