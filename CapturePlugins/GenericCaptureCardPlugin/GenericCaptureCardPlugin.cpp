#include "pch.h"
#include "GenericCaptureCardPlugin.h"
#include "Controller.g.cpp"
#include "ControllerFactory.g.cpp"
#include <filesystem>

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
    winrt::IController ControllerFactory::CreateController(winrt::ILogger const& logger)
    {
        return winrt::make<Controller>(logger);
    }

    Controller::Controller()
    {
        // Throw - callers should explicitly instantiate through the factory
        throw winrt::hresult_illegal_method_call();
    }

    Controller::Controller(winrt::ILogger const& logger) : m_logger(logger)
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
            m_displayInputs.push_back(*make_self<DisplayInput>(m_usbIdFromConfigFile, m_logger));
        }
        else // Attempt to get all possible inputs
        {
            hstring cameraId;
            auto captureDevices = DeviceInformation::FindAllAsync(DeviceClass::VideoCapture).get();
            for (auto&& captureDevice : captureDevices)
            {
                m_displayInputs.push_back(*make_self<DisplayInput>(captureDevice.Id(), m_logger));
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
            m_logger.LogError(Name() + L" was provided configuration data, but was unable to parse it. Error: " + ex.message());
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

    DisplayInput::DisplayInput(hstring deviceId, winrt::ILogger const& logger) :
        m_deviceId(deviceId),
        m_logger(logger)
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

            m_capture = make_self<DisplayCapture>(photo.Frame(), m_logger, extendedProps);
        }
        catch (...)
        {
            m_logger.LogError(L"Unable to get pixels for input: " + Name());
            m_capture = nullptr;
            return nullptr;
        }

        return *m_capture;
    }

    DisplayCapture::DisplayCapture(CapturedFrame frame, winrt::ILogger const& logger, winrt::IMap<winrt::hstring, winrt::IInspectable> extendedProps) :
        m_logger(logger), m_extendedProps(extendedProps)
    {
        // Ensure that we can actually read the provided frame
        if (!frame.CanRead())
        {
            m_logger.LogError(L"Cannot read pixel data from frame.");
            throw winrt::hresult_invalid_argument();
        }

        m_frameData = FrameData(m_logger);

        // Copy the new data over to the FrameData object
        auto capturedFrameBitmap = SoftwareBitmap::Convert(frame.SoftwareBitmap(), BitmapPixelFormat::Rgba8);

        m_frameData.Resolution({capturedFrameBitmap.PixelWidth(), capturedFrameBitmap.PixelHeight()});

        FrameFormatDescription desc{0};
        desc.BitsPerPixel = 32;
        desc.Stride = capturedFrameBitmap.PixelWidth() * 4;
        desc.PixelFormat = static_cast<DirectXPixelFormat>(BitmapPixelFormat::Rgba8);
        m_frameData.FormatDescription(desc);

        auto buffer = Buffer(static_cast<uint32_t>(frame.Size()));
        capturedFrameBitmap.CopyToBuffer(buffer);
        m_frameData.Data(buffer);
    }

    bool DisplayCapture::CompareCaptureToPrediction(hstring name, MicrosoftDisplayCaptureTools::ConfigurationTools::IPredictionData prediction)
    {
        auto predictedFrame = prediction.FrameData();

        // Compare descriptions
        auto predictedFrameDesc = predictedFrame.FormatDescription();
        auto capturedFrameDesc = m_frameData.FormatDescription();
        if (m_frameData.Data().Length() < predictedFrame.Data().Length())
        {
            m_logger.LogError(
                winrt::hstring(L"Capture should be at least as large as prediction") +
                std::to_wstring(m_frameData.Data().Length()) +
                L", Predicted=" + std::to_wstring(predictedFrame.Data().Length()));
        }
        else if (0 != memcmp(m_frameData.Data().data(), predictedFrame.Data().data(), predictedFrame.Data().Length()))
        {
            m_logger.LogWarning(L"Capture did not exactly match prediction! Attempting comparison with tolerance.");
            {
                auto filename = name + L"_Captured.bin";
                auto folder = winrt::StorageFolder::GetFolderFromPathAsync(std::filesystem::current_path().c_str()).get();
                auto file = folder.CreateFileAsync(filename, winrt::CreationCollisionOption::GenerateUniqueName).get();
                auto stream = file.OpenAsync(winrt::FileAccessMode::ReadWrite).get();
                stream.WriteAsync(m_frameData.Data()).get();
                stream.FlushAsync().get();
                stream.Close();

                m_logger.LogNote(L"Dumping captured data here: " + filename);
            }
            {
                auto filename = name + L"_Predicted.bin";
                auto folder = winrt::StorageFolder::GetFolderFromPathAsync(std::filesystem::current_path().c_str()).get();
                auto file = folder.CreateFileAsync(filename, winrt::CreationCollisionOption::GenerateUniqueName).get();
                auto stream = file.OpenAsync(winrt::FileAccessMode::ReadWrite).get();
                stream.WriteAsync(predictedFrame.Data()).get();
                stream.FlushAsync().get();
                stream.Close();

                m_logger.LogNote(L"Dumping captured data here: " + filename);
            }

            struct PixelStruct
            {
                uint8_t r, g, b, a;
            };

            auto differenceCount = 0;

            PixelStruct* cap = reinterpret_cast<PixelStruct*>(m_frameData.Data().data());
            PixelStruct* pre = reinterpret_cast<PixelStruct*>(predictedFrame.Data().data());

            // Comparing pixel for pixel takes a very long time at the moment - so let's compare stochastically
            const int samples = 10000;
            const int pixelCount = m_frameData.Resolution().Width * m_frameData.Resolution().Height;
            for (auto i = 0; i < samples; i++)
            {
                auto index = rand() % pixelCount;

                if (cap[index].r != pre[index].r || cap[index].g != pre[index].g || cap[index].b != pre[index].b)
                {
                    differenceCount++;
                }
            }

            float diff = (float)differenceCount / (float)pixelCount;
            if (diff > 0.10f)
            {
                std::wstring msg;
                std::format_to(std::back_inserter(msg), "\n\tMatch = %2.2f\n\n", diff);
                m_logger.LogError(msg);

                return false;
            }
        }

        return true;
    }

    winrt::IFrameData DisplayCapture::GetFrameData()
    {
        return m_frameData;
    }

    winrt::IMapView<winrt::hstring, winrt::IInspectable> DisplayCapture::ExtendedProperties()
    {
        return m_extendedProps.GetView();
    }
}