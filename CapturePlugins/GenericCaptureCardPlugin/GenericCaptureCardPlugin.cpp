#include "pch.h"
#include "GenericCaptureCardPlugin.h"
#include "Controller.g.cpp"
#include "ControllerFactory.g.cpp"
#include <filesystem>

// included for translations of the half data format of the prediction
#include <DirectXPackedVector.h>

namespace winrt
{
    using namespace Windows::Foundation;
    using namespace Windows::Foundation::Collections;
    using namespace Windows::Data::Json;
    using namespace Windows::Media::Capture;
    using namespace Windows::Media::Capture::Core;
    using namespace Windows::Media::Capture::Frames;
    using namespace Windows::Media::MediaProperties;
    using namespace Windows::Storage;
    using namespace Windows::Storage::Streams;
    using namespace Windows::Graphics;
    using namespace Windows::Graphics::Imaging;
    using namespace Windows::Graphics::DirectX;
    using namespace Windows::Devices::Enumeration;
    using namespace Windows::Devices::Display::Core;


    using namespace MicrosoftDisplayCaptureTools::CaptureCard;
    using namespace MicrosoftDisplayCaptureTools::Display;
    using namespace MicrosoftDisplayCaptureTools::Framework;
}

struct __declspec(uuid("5b0d3235-4dba-4d44-865e-8f1d0e4fd04d")) __declspec(novtable) IMemoryBufferByteAccess : ::IUnknown
{
    virtual HRESULT __stdcall GetBuffer(uint8_t** value, uint32_t* capacity) = 0;
};

namespace winrt::GenericCaptureCardPlugin::implementation
{
    winrt::IController ControllerFactory::CreateController()
    {
        return winrt::make<Controller>();
    }

    Controller::Controller()
    {
    }

    hstring Controller::Name()
    {
        return L"GenericCapture Plugin";
    }

    com_array<IDisplayInput> Controller::EnumerateDisplayInputs()
    {
        // Only try and get the input specified by config file
        if (m_usbIdFromConfigFile != L"")
        {
            m_displayInputs.push_back(*make_self<DisplayInput>(m_usbIdFromConfigFile));
        }
        else // Attempt to get all possible inputs
        {
            hstring cameraId;
            auto captureDevices = DeviceInformation::FindAllAsync(DeviceClass::VideoCapture).get();
            for (auto&& captureDevice : captureDevices)
            {
                m_displayInputs.push_back(*make_self<DisplayInput>(captureDevice.Id()));
            }
        }


        return com_array<IDisplayInput>(m_displayInputs);
    }

    void Controller::SetConfigData(winrt::IJsonValue data)
    {
        // The JSON object returned here is the "settings" sub-object for this plugin. It's definition is capture card plugin-specific.
        if (!data || data.ValueType() == winrt::JsonValueType::Null)
        {
            return;
        }

        try
        {
            auto jsonObject = data.GetObjectW();
            m_usbIdFromConfigFile = jsonObject.GetNamedString(L"UsbId");

            // Make sure we only get the capture device specified if going this route
            m_displayInputs.clear();

        }
        catch (winrt::hresult_error const& ex)
        {
            Logger().LogError(Name() + L" was provided configuration data, but was unable to parse it. Error: " + ex.message());
        }
    }

    bool CaptureCapabilities::CanReturnRawFramesToHost()
    {
        return false;
    }

    bool CaptureCapabilities::CanReturnFramesToHost()
    {
        return true;
    }

    bool CaptureCapabilities::CanCaptureFrameSeries()
    {
        return false;
    }

    bool CaptureCapabilities::CanHotPlug()
    {
        return false;
    }

    bool CaptureCapabilities::CanConfigureEDID()
    {
        return false;
    }

    bool CaptureCapabilities::CanConfigureDisplayID()
    {
        return false;
    }

    uint32_t CaptureCapabilities::GetMaxDescriptorSize()
    {
        // This should never be called given that neither CanConfigureEDID nor CanConfigureDisplayID
        // return true.
        throw hresult_not_implemented();
    }

    CaptureTriggerType CaptureTrigger::Type()
    {
        return m_type;
    }

    void CaptureTrigger::Type(CaptureTriggerType type)
    {
        m_type = type;
    }

    uint32_t CaptureTrigger::TimeToCapture()
    {
        return m_time;
    }

    void CaptureTrigger::TimeToCapture(uint32_t time)
    {
        m_time = time;
    }

    DisplayInput::DisplayInput(hstring deviceId) :
        m_deviceId(deviceId)
    {
        hstring cameraId;
        auto captureDevices = DeviceInformation::FindAllAsync(DeviceClass::VideoCapture).get();
        for (auto&& captureDevice : captureDevices)
        {
            if (deviceId == captureDevice.Id())
            {
                // Capture Camera Found!
                m_deviceFriendlyName = captureDevice.Name();
                break;
            }
        }

        m_captureCapabilities = make_self<CaptureCapabilities>();
        m_captureTrigger = make_self<CaptureTrigger>();
    }

    hstring DisplayInput::Name()
    {
        return m_deviceFriendlyName;
    }

    void DisplayInput::FinalizeDisplayState()
    {
        MediaCaptureInitializationSettings mediaCaptureInitSettings;
        mediaCaptureInitSettings.VideoDeviceId(m_deviceId);
        mediaCaptureInitSettings.StreamingCaptureMode(StreamingCaptureMode::Video);

        m_mediaCapture = MediaCapture();

        // Initialize the capture object and block until complete
        m_mediaCapture.InitializeAsync(mediaCaptureInitSettings).get();
    }

    ICaptureCapabilities DisplayInput::GetCapabilities()
    {
        return *m_captureCapabilities;
    }

    ICaptureTrigger DisplayInput::GetCaptureTrigger()
    {
        return *m_captureTrigger;
    }

    IDisplayCapture DisplayInput::CaptureFrame()
    {
        try
        {
            auto cap = m_mediaCapture.PrepareLowLagPhotoCaptureAsync(ImageEncodingProperties::CreateUncompressed(MediaPixelFormat::Bgra8));
            auto photo = cap.get().CaptureAsync().get();

            // Add any extended properties that aren't directly exposed through the IDisplayCapture* interfaces
            auto extendedProps = winrt::multi_threaded_observable_map<winrt::hstring, winrt::IInspectable>();
            extendedProps.Insert(L"Timestamp", winrt::box_value(winrt::DateTime(winrt::clock::now())));

            m_capture = make_self<DisplayCapture>(photo.Frame(), extendedProps);
        }
        catch (...)
        {
            Logger().LogError(L"Unable to get pixels for input: " + Name());
            m_capture = nullptr;
            return nullptr;
        }

        return *m_capture;
    }

    DisplayCapture::DisplayCapture(CapturedFrame frame, winrt::IMap<winrt::hstring, winrt::IInspectable> extendedProps) :
        m_extendedProps(extendedProps)
    {
        // TODO: this needs to support multiple frames in a set.

        // Ensure that we can actually read the provided frame
        if (!frame.CanRead())
        {
            Logger().LogError(L"Cannot read pixel data from frame.");
            throw winrt::hresult_invalid_argument();
        }

        m_frameSet = winrt::make<FrameSet>();
        auto newFrame = winrt::make<Frame>();

        // Copy the new data over to the FrameData object
        auto capturedFrameBitmap = SoftwareBitmap::Convert(frame.SoftwareBitmap(), BitmapPixelFormat::Rgba8);
        newFrame.as<Frame>()->SetImageApproximation(SoftwareBitmap::Copy(capturedFrameBitmap));

        // Populate the frame format description, which for this capture card can only be 24bpc SDR RGB444.
        auto wireFormat = winrt::DisplayWireFormat(
            winrt::DisplayWireFormatPixelEncoding::Rgb444,
            24, // bits per pixel (minus alpha)
            winrt::DisplayWireFormatColorSpace::BT709,
            winrt::DisplayWireFormatEotf::Sdr,
            winrt::DisplayWireFormatHdrMetadata::None);

        newFrame.as<Frame>()->DataFormat(wireFormat);
        newFrame.as<Frame>()->Resolution({capturedFrameBitmap.PixelWidth(), capturedFrameBitmap.PixelHeight()});

        auto buffer = Buffer(static_cast<uint32_t>(frame.Size()));
        capturedFrameBitmap.CopyToBuffer(buffer);
        newFrame.as<Frame>()->SetBuffer(buffer);

        m_frameSet.Frames().Append(newFrame);
    }

    bool DisplayCapture::CompareCaptureToPrediction(hstring name, MicrosoftDisplayCaptureTools::Framework::IRawFrameSet prediction)
    {
        // TODO: this needs to support multiple frames in a set.
        auto predictedFrame = prediction.Frames().GetAt(0);
        auto capturedFrame = m_frameSet.Frames().GetAt(0);

        // Compare the frame resolutions
        auto predictedFrameRes = predictedFrame.Resolution();
        auto capturedFrameRes = capturedFrame.Resolution();
        if (predictedFrameRes.Width != capturedFrameRes.Width || predictedFrameRes.Height != capturedFrameRes.Height)
        {
            Logger().LogError(
				winrt::hstring(L"Capture resolution did not match prediction") +
				std::to_wstring(capturedFrameRes.Width) + L"x" + std::to_wstring(capturedFrameRes.Height) +
				L", Predicted=" + std::to_wstring(predictedFrameRes.Width) + L"x" + std::to_wstring(predictedFrameRes.Height));
			return false;
		}

        // Compare the frame formats
        auto predictedFrameFormat = predictedFrame.DataFormat();
        auto capturedFrameFormat = capturedFrame.DataFormat();
        if (predictedFrameFormat.PixelEncoding() != capturedFrameFormat.PixelEncoding() ||
            predictedFrameFormat.ColorSpace() != capturedFrameFormat.ColorSpace() ||
            predictedFrameFormat.Eotf() != capturedFrameFormat.Eotf() ||
            predictedFrameFormat.HdrMetadata() != capturedFrameFormat.HdrMetadata())
        {
            Logger().LogError(winrt::hstring(L"Capture format did not match prediction"));
			return false;
		}

        // At this point, both frames should be identical in terms of resolution and format. Now we can compare the actual pixel data.
        if (0 != memcmp(predictedFrame.Data().data(), capturedFrame.Data().data(), predictedFrame.Data().Length()))
        {
            Logger().LogWarning(L"Capture did not exactly match prediction! Attempting comparison with tolerance.");

            // TODO: remove this saving logic, it isn't needed and is only for debugging
            {
                auto filename = name + L"_Captured.bin";
                auto folder = winrt::StorageFolder::GetFolderFromPathAsync(std::filesystem::current_path().c_str()).get();
                auto file = folder.CreateFileAsync(filename, winrt::CreationCollisionOption::GenerateUniqueName).get();
                auto stream = file.OpenAsync(winrt::FileAccessMode::ReadWrite).get();
                stream.WriteAsync(capturedFrame.Data()).get();
                stream.FlushAsync().get();
                stream.Close();

                Logger().LogNote(L"Dumping captured data here: " + filename);
            }
            {
                auto filename = name + L"_Predicted.bin";
                auto folder = winrt::StorageFolder::GetFolderFromPathAsync(std::filesystem::current_path().c_str()).get();
                auto file = folder.CreateFileAsync(filename, winrt::CreationCollisionOption::GenerateUniqueName).get();
                auto stream = file.OpenAsync(winrt::FileAccessMode::ReadWrite).get();
                stream.WriteAsync(predictedFrame.Data()).get();
                stream.FlushAsync().get();
                stream.Close();

                Logger().LogNote(L"Dumping captured data here: " + filename);
            }

            // This capture plugin only supports 8bpc SDR RGB444, so we can do a direct comparison of the pixel data.
            struct PixelStruct
            {
                uint8_t r, g, b, a;
            };

            struct PixelStruct16
            {
                ::DirectX::PackedVector::HALF r, g, b, a;
			};

            PixelStruct* cap = reinterpret_cast<PixelStruct*>(capturedFrame.Data().data());
            PixelStruct16* pre = reinterpret_cast<PixelStruct16*>(predictedFrame.Data().data());

            auto differenceCount = 0;

            struct PixelDiff
            {
				float r, g, b;
            } PeakDiffPercentage{}, AverageDiffPercentage{};

            const int threadCount = 8;
            const int pixelCount = capturedFrameRes.Width * capturedFrameRes.Height;
            auto threads = std::vector<std::thread>(threadCount);
            auto threadPeakDiffs = std::vector<PixelDiff>(threadCount);
            auto threadTotalDiffs = std::vector<PixelDiff>(threadCount);
            for (auto i = 0; i < threadCount; i++)
            {
                threads[i] = std::thread([&, i]()
                    {
                        auto start = i * pixelCount / threadCount;
                        auto end = (i + 1) * pixelCount / threadCount;

                        for (auto j = start; j < end; j++)
                        {
                            PixelStruct translatedPrediction = {
                                static_cast<uint8_t>(::DirectX::PackedVector::XMConvertHalfToFloat(pre[j].r) * 255.0f),
								static_cast<uint8_t>(::DirectX::PackedVector::XMConvertHalfToFloat(pre[j].g) * 255.0f),
								static_cast<uint8_t>(::DirectX::PackedVector::XMConvertHalfToFloat(pre[j].b) * 255.0f),
								255};

                            if (cap[j].r != translatedPrediction.r || 
                                cap[j].g != translatedPrediction.g ||
                                cap[j].b != translatedPrediction.b)
                            {
                                differenceCount++;

                                PixelDiff diff = {
                                    abs(cap[j].r - translatedPrediction.r) / 255.0f,
                                    abs(cap[j].g - translatedPrediction.g) / 255.0f,
                                    abs(cap[j].b - translatedPrediction.b) / 255.0f};

                                threadPeakDiffs[i] = {
									max(threadPeakDiffs[i].r, diff.r),
									max(threadPeakDiffs[i].g, diff.g),
									max(threadPeakDiffs[i].b, diff.b)};

								threadTotalDiffs[i].r += diff.r;
                                threadTotalDiffs[i].g += diff.g;
                                threadTotalDiffs[i].b += diff.b;
							}
						}
					});
			}

            // Join the comparing threads
            for (auto& thread : threads)
            {
				thread.join();
			}

			// Combine the results from the threads
			for (auto i = 0; i < threadCount; i++)
			{
				PeakDiffPercentage.r = max(PeakDiffPercentage.r, threadPeakDiffs[i].r);
				PeakDiffPercentage.g = max(PeakDiffPercentage.g, threadPeakDiffs[i].g);
				PeakDiffPercentage.b = max(PeakDiffPercentage.b, threadPeakDiffs[i].b);

				AverageDiffPercentage.r += threadTotalDiffs[i].r;
				AverageDiffPercentage.g += threadTotalDiffs[i].g;
				AverageDiffPercentage.b += threadTotalDiffs[i].b;
			}

			AverageDiffPercentage.r /= pixelCount;
			AverageDiffPercentage.g /= pixelCount;
			AverageDiffPercentage.b /= pixelCount;

			std::wstring msg;
			std::format_to(std::back_inserter(msg), L"\n\tPeak Diff = %2.2f%%, %2.2f%%, %2.2f%%\n", PeakDiffPercentage.r * 100.0f, PeakDiffPercentage.g * 100.0f, PeakDiffPercentage.b * 100.0f);
			std::format_to(std::back_inserter(msg), L"\tAverage Diff = %2.2f%%, %2.2f%%, %2.2f%%\n", AverageDiffPercentage.r * 100.0f, AverageDiffPercentage.g * 100.0f, AverageDiffPercentage.b * 100.0f);
			std::format_to(std::back_inserter(msg), L"\tTotal Diff = %d\n\n", differenceCount);
			Logger().LogError(msg);

			// If the difference is too great, then the capture is considered a failure.
			if (PeakDiffPercentage.r > 0.10f || PeakDiffPercentage.g > 0.10f || PeakDiffPercentage.b > 0.10f)
			{
				return false;
			}
        }

        return true;
    }

    winrt::IRawFrameSet DisplayCapture::GetFrameData()
    {
        return m_frameSet;
    }

    winrt::IMapView<winrt::hstring, winrt::IInspectable> DisplayCapture::ExtendedProperties()
    {
        return m_extendedProps.GetView();
    }

    Frame::Frame()
    {
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

    FrameSet::FrameSet()
    {
        m_frames = winrt::single_threaded_vector<winrt::IRawFrame>();
    }

    winrt::IVector<winrt::IRawFrame> FrameSet::Frames()
    {
        return m_frames;
    }

    winrt::IMap<winrt::hstring, winrt::IInspectable> FrameSet::Properties()
    {
        return m_properties;
    }
}