#include "pch.h"
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

using namespace winrt::MicrosoftDisplayCaptureTools::GenericCaptureCardPlugin::DataProcessing;

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
    }

    hstring DisplayInput::Name()
    {
        return m_deviceFriendlyName;
    }

    void DisplayInput::FinalizeDisplayState()
    {
        auto mediaCaptureInitSettings = MediaCaptureInitializationSettings(); 
        mediaCaptureInitSettings.VideoDeviceId(m_deviceId);
        mediaCaptureInitSettings.PhotoCaptureSource(PhotoCaptureSource::Photo);
        mediaCaptureInitSettings.StreamingCaptureMode(StreamingCaptureMode::Video);

        m_mediaCapture = MediaCapture();

        // Initialize the capture object and block until complete
        m_mediaCapture.InitializeAsync(mediaCaptureInitSettings).get();
    }

    ICaptureCapabilities DisplayInput::GetCapabilities()
    {
        return winrt::make<CaptureCapabilities>();
    }

    ICaptureTrigger DisplayInput::GetCaptureTrigger()
    {
        return winrt::make<CaptureTrigger>();
    }

    void DisplayInput::SetDescriptor(IMonitorDescriptor descriptor)
    {
        Logger().LogAssert(L"SetDescriptor not available for this capture card.");

		// This should never be called given that CanConfigureEDID returns false.
		throw hresult_not_implemented();
	}

    IDisplayCapture DisplayInput::CaptureFrame()
    {
        std::this_thread::sleep_for(std::chrono::seconds(2));

        try
        {
            auto cap = m_mediaCapture.PrepareLowLagPhotoCaptureAsync(ImageEncodingProperties::CreateUncompressed(MediaPixelFormat::Bgra8));
            auto photo = cap.get().CaptureAsync().get();

            // Add any extended properties that aren't directly exposed through the IDisplayCapture* interfaces
            auto extendedProps = winrt::multi_threaded_observable_map<winrt::hstring, winrt::IInspectable>();
            extendedProps.Insert(L"Timestamp", winrt::box_value(winrt::DateTime(winrt::clock::now())));

            return winrt::make<DisplayCapture>(photo.Frame(), extendedProps);
        }
        catch (...)
        {
            Logger().LogError(L"Unable to get pixels for input: " + Name());
            return nullptr;
        }
    }
}