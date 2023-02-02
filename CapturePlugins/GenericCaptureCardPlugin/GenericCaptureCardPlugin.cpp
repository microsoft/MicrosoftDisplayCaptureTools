#include "pch.h"
#include "GenericCaptureCardPlugin.h"
#include "Controller.g.cpp"
#include "ControllerFactory.g.cpp"

namespace winrt
{
    using namespace Windows::Foundation;
    using namespace Windows::Foundation::Collections;
    using namespace Windows::Data::Json;
    using namespace Windows::Media::Capture;
    using namespace Windows::Media::Capture::Core;
    using namespace Windows::Media::Capture::Frames;
    using namespace Windows::Media::MediaProperties;
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

        FrameDataDescription desc{0};
        desc.BitsPerPixel = 32;
        desc.Stride = capturedFrameBitmap.PixelWidth() * 3; // There is no padding with this capture
        desc.PixelFormat = static_cast<DirectXPixelFormat>(BitmapPixelFormat::Rgba8);
        m_frameData.FormatDescription(desc);

        auto buffer = Buffer(static_cast<uint32_t>(frame.Size()));
        capturedFrameBitmap.CopyToBuffer(buffer);
        m_frameData.Data(buffer);
    }

    bool DisplayCapture::CompareCaptureToPrediction(hstring name, MicrosoftDisplayCaptureTools::Display::IDisplayEnginePrediction prediction)
    {
        auto predictedFrame = prediction.GetFrameData();
        
        // Compare descriptions
        auto predictedFrameDesc = predictedFrame.FormatDescription();
        auto capturedFrameDesc = m_frameData.FormatDescription();

        /*

        // Convert limited range to full range
        capturedR = capturedR < 16 ? 0 : capturedR > 235 ? 235 : (uint8_t)((float)(capturedR - 16) * 1.164f);
        capturedG = capturedG < 16 ? 0 : capturedG > 235 ? 235 : (uint8_t)((float)(capturedG - 16) * 1.164f);
        capturedB = capturedB < 16 ? 0 : capturedB > 235 ? 235 : (uint8_t)((float)(capturedB - 16) * 1.164f);

        m_logger.LogNote(std::format(
            L"Captured pixel ({},{},{}) - Expected ({},{},{})",
            capturedR,
            capturedG,
            capturedB,
            predictBuffer.data()[0],
            predictBuffer.data()[1],
            predictBuffer.data()[2]));

        //
        // Compare the two images. In some capture cards this can be done on the capture device itself. In this generic
        // plugin only RGB8 is supported.
        //
        // TODO: right now this is only comparing a single pixel for speed reasons - both of the buffers are fully available here.
        //
        if (ColorChannelTolerance < static_cast<uint8_t>(fabsf((float)capturedR - (float)predictBuffer.data()[0])) ||
            ColorChannelTolerance < static_cast<uint8_t>(fabsf((float)capturedG - (float)predictBuffer.data()[1])) ||
            ColorChannelTolerance < static_cast<uint8_t>(fabsf((float)capturedB - (float)predictBuffer.data()[2])))
        {
            m_logger.LogError(L"Image comparison failed.");
            return false;
        }
        */

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