#include "pch.h"
#include "GenericCaptureCardPlugin.h"
#include "Controller.g.cpp"

#include "winrt\MicrosoftDisplayCaptureTools.Display.h"

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
    using namespace Windows::Graphics::Imaging;
    using namespace Windows::Devices::Enumeration;

    using namespace MicrosoftDisplayCaptureTools::CaptureCard;
    using namespace MicrosoftDisplayCaptureTools::Display;
}

struct __declspec(uuid("5b0d3235-4dba-4d44-865e-8f1d0e4fd04d")) __declspec(novtable) IMemoryBufferByteAccess : ::IUnknown
{
    virtual HRESULT __stdcall GetBuffer(uint8_t** value, uint32_t* capacity) = 0;
};

namespace winrt::GenericCaptureCardPlugin::implementation
{
    Controller::Controller()
    {
    }
    hstring Controller::Name()
    {
        return L"GenericCapture";
    }
    com_array<IDisplayInput> Controller::EnumerateDisplayInputs()
    {
        m_displayInput = *make_self<DisplayInput>(m_deviceId);

        std::vector<IDisplayInput> inputs;
        inputs.push_back(m_displayInput);
        return com_array<IDisplayInput>(inputs);
    }
    void Controller::SetConfigData(winrt::IJsonValue data)
    {
        // The JSON object returned here is the "settings" sub-object for this plugin. It's definition is capture card plugin-specific.
        if (!data || data.ValueType() == winrt::JsonValueType::Null)
        {
            // We don't have anything that can identify the capture device
            // TODO: log this case - potentially we can fall back to using the first camera device?
            throw winrt::hresult_invalid_argument();
        }

        auto jsonObject = data.GetObjectW();
        m_deviceId = jsonObject.GetNamedString(L"UsbId");

        // TODO: log that the device ID has been read
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
            std::wstring_view name = captureDevice.Id();
            if (name.find(m_deviceId) != std::wstring::npos)
            {
                // Capture Camera Found!
                cameraId = captureDevice.Id();
                break;
            }
        }

        MediaCaptureInitializationSettings mediaCaptureInitSettings;
        mediaCaptureInitSettings.VideoDeviceId(cameraId);
        mediaCaptureInitSettings.StreamingCaptureMode(StreamingCaptureMode::Video);

        m_mediaCapture = MediaCapture();

        // Initialize the capture object and block until complete
        m_mediaCapture.InitializeAsync(mediaCaptureInitSettings).get();

        m_captureCapabilities = make_self<CaptureCapabilities>();
        m_captureTrigger = make_self<CaptureTrigger>();
    }
    hstring DisplayInput::Name()
    {
        hstring ret = L"Input 1";
        return ret;
    }
    void DisplayInput::FinalizeDisplayState()
    {
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
        auto cap = m_mediaCapture.PrepareLowLagPhotoCaptureAsync(ImageEncodingProperties::CreateUncompressed(MediaPixelFormat::Bgra8));
        auto photo = cap.get().CaptureAsync().get();

        m_capture = make_self<DisplayCapture>(photo.Frame());
        return *m_capture;
    }

    DisplayCapture::DisplayCapture(CapturedFrame frame)
    {                
        // Mirror the pixel data over to this object's storage. 
        if (!frame.CanRead())
        {
            // For some reason we are unable to read the frame, log this and throw
            // TODO - add logging
        }

        auto bitmap = frame.SoftwareBitmap();
        m_bitmap = SoftwareBitmap::Convert(bitmap, BitmapPixelFormat::Rgba8);
    }
    void DisplayCapture::CompareCaptureToPrediction(hstring name, MicrosoftDisplayCaptureTools::Display::IDisplayEnginePrediction prediction)
    {
        auto captureBuffer = m_bitmap.LockBuffer(BitmapBufferAccessMode::Read).CreateReference();
        auto predictBuffer = prediction.GetBitmap().LockBuffer(BitmapBufferAccessMode::Read).CreateReference();
        
        //
        // Compare the two images. In some capture cards this can be done on the capture device itself. In this generic
        // plugin only RGB8 is supported.
        // 
        // TODO: this needs to handle multiple formats 
        // TODO: right now this is only comparing a single pixel for speed reasons - both of the buffers are fully available here.
        // TODO: allow saving the diff
        //
        if (ColorChannelTolerance < static_cast<uint8_t>(fabsf((float)captureBuffer.data()[0] - (float)predictBuffer.data()[0])) ||
            ColorChannelTolerance < static_cast<uint8_t>(fabsf((float)captureBuffer.data()[1] - (float)predictBuffer.data()[1])) ||
            ColorChannelTolerance < static_cast<uint8_t>(fabsf((float)captureBuffer.data()[2] - (float)predictBuffer.data()[2])))
        {
            // TODO: replace with actual logging
            printf("Captured  - %d, %d, %d\nPredicted - %d, %d, %d\n\n",
                captureBuffer.data()[0], captureBuffer.data()[1], captureBuffer.data()[2],
                predictBuffer.data()[0], predictBuffer.data()[1], predictBuffer.data()[2]);

            throw winrt::hresult_error();
        }
    }
    IMemoryBufferReference DisplayCapture::GetRawPixelData()
    {
        auto buffer = m_bitmap.LockBuffer(BitmapBufferAccessMode::Read);
        return buffer.CreateReference();
    }
}
