#include "pch.h"

namespace winrt 
{
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Windows::Foundation::Collections;
    using namespace winrt::Windows::Devices::Enumeration;
    using namespace winrt::Windows::Devices::Usb;
    using namespace winrt::Windows::Graphics::Imaging;
    using namespace winrt::Windows::Devices::Display;
    using namespace winrt::Windows::Devices::Display::Core;
    using namespace winrt::Windows::Storage;
    using namespace winrt::Windows::Storage::Streams;
    using namespace winrt::MicrosoftDisplayCaptureTools::Framework;
}
using namespace IteIt68051Plugin;

namespace winrt::TanagerPlugin::implementation
{
const unsigned char it68051i2cAddress = 0x48;

TanagerDevice::TanagerDevice(winrt::param::hstring deviceId, winrt::ILogger const& logger) :
    m_logger(logger),
    m_usbDevice(nullptr),
    m_deviceId(deviceId),
    hdmiChip(
        [&](uint8_t address, uint8_t value) // I2C write
        { m_pDriver->writeRegisterByte(it68051i2cAddress, address, value); },
        [&](uint8_t address) // I2C read
        { return m_pDriver->readRegisterByte(it68051i2cAddress, address); })
	{
		m_usbDevice = UsbDevice::FromIdAsync(deviceId).get();
		if (m_usbDevice == nullptr)
		{
			throw_hresult(E_FAIL);
		}
		m_fpga.SetUsbDevice(m_usbDevice);

		m_pDriver = std::make_shared<I2cDriver>(m_usbDevice);

        m_fpga.SysReset(); // Blocks until FPGA is ready

		hdmiChip.Initialize();

        m_logger.LogNote(L"Initializing Tanager Device: " + m_deviceId);
	}

	TanagerDevice::~TanagerDevice()
	{
	}

	std::vector<MicrosoftDisplayCaptureTools::CaptureCard::IDisplayInput> TanagerDevice::EnumerateDisplayInputs()
	{
		return std::vector<MicrosoftDisplayCaptureTools::CaptureCard::IDisplayInput>
		{
			winrt::make<TanagerDisplayInput>(this->weak_from_this(), TanagerDisplayInputPort::hdmi, m_logger),
			winrt::make<TanagerDisplayInput>(this->weak_from_this(), TanagerDisplayInputPort::displayPort, m_logger),
		};
	}

	void TanagerDevice::TriggerHdmiCapture()
	{
        m_logger.LogAssert(L"TriggerHdmiCapture not currently implemented.");
	}

	void TanagerDevice::FpgaWrite(unsigned short address, std::vector<byte> data)
	{
		return m_fpga.Write(address, data);
	}

	std::vector<byte> TanagerDevice::FpgaRead(unsigned short address, UINT16 size)
	{
		return m_fpga.Read(address, size);
	}

	std::vector<byte> TanagerDevice::ReadEndPointData(UINT32 dataSize)
	{
		return m_fpga.ReadEndPointData(dataSize);
	}

	void TanagerDevice::FlashFpgaFirmware(winrt::hstring filePath)
	{
        m_fpga.FlashFpgaFirmware(filePath);
	}

	void TanagerDevice::FlashFx3Firmware(winrt::hstring filePath)
	{
        m_fpga.FlashFx3Firmware(filePath);
	}

	FirmwareVersionInfo TanagerDevice::GetFirmwareVersionInfo()
	{
		return m_fpga.GetFirmwareVersionInfo();
	}

    IteIt68051Plugin::VideoTiming TanagerDevice::GetVideoTiming()
    {
        return hdmiChip.GetVideoTiming();
    }

    winrt::hstring TanagerDevice::GetDeviceId()
    {
        return m_deviceId;
    }

    winrt::Windows::Foundation::IAsyncAction TanagerDevice::UpdateFirmwareAsync()
    {
        co_await winrt::resume_background();

        FlashFpgaFirmware(FpgaFirmwareFileName);
        FlashFx3Firmware(Fx3FirmwareFileName);
    }

    MicrosoftDisplayCaptureTools::CaptureCard::ControllerFirmwareState TanagerDevice::GetFirmwareState()
    {
        auto versionInfo = GetFirmwareVersionInfo();
        if (versionInfo.GetFpgaFirmwareVersion() < MinimumFpgaVersion || versionInfo.GetFx3FirmwareVersion() < MinimumFx3Version)
        {
            return MicrosoftDisplayCaptureTools::CaptureCard::ControllerFirmwareState::UpdateRequired;
        }
        else
        {
            return MicrosoftDisplayCaptureTools::CaptureCard::ControllerFirmwareState::UpToDate;
        }
    }

	TanagerDisplayInput::TanagerDisplayInput(std::weak_ptr<TanagerDevice> parent, TanagerDisplayInputPort port, winrt::ILogger const& logger) :
        m_parent(parent), m_port(port), m_logger(logger)
    {
    }

	hstring TanagerDisplayInput::Name()
	{
		switch (m_port)
		{
		case TanagerDisplayInputPort::hdmi:
			return L"HDMI";
		case TanagerDisplayInputPort::displayPort:
			return L"DisplayPort";
        default: 
            m_logger.LogAssert(L"Invalid port chosen.");
            return L"";
		}
    }

    void TanagerDisplayInput::SetDescriptor(MicrosoftDisplayCaptureTools::Framework::IMonitorDescriptor descriptor)
    {
        switch (m_port)
        {
        case TanagerDisplayInputPort::hdmi:
        {
            if (descriptor.Type() != MicrosoftDisplayCaptureTools::Framework::MonitorDescriptorType::EDID)
            {
                m_logger.LogAssert(L"Only EDID descriptors are currently supported.");
            }

            auto edidDataView = descriptor.Data();
            std::vector<byte> edid(edidDataView.begin(), edidDataView.end());

            SetEdid(edid);
            break;
        }
        case TanagerDisplayInputPort::displayPort:
        default:
        {
            m_logger.LogAssert(L"DisplayPort input does not currently support setting descriptors.");
        }
        }
    }

    MicrosoftDisplayCaptureTools::CaptureCard::ICaptureTrigger TanagerDisplayInput::GetCaptureTrigger()
    {
        return MicrosoftDisplayCaptureTools::CaptureCard::ICaptureTrigger();
    }

	MicrosoftDisplayCaptureTools::CaptureCard::ICaptureCapabilities TanagerDisplayInput::GetCapabilities()
	{
        auto caps = winrt::make<TanagerCaptureCapabilities>(m_port);
        return caps;
    }

    MicrosoftDisplayCaptureTools::CaptureCard::IDisplayCapture TanagerDisplayInput::CaptureFrame()
    {
        auto parent = m_parent.lock();
        if (!parent)
        {
            m_logger.LogAssert(L"Cannot obtain reference to Tanager device.");
        }

        // Capture frame in DRAM
        parent->FpgaWrite(0x20, std::vector<byte>({0}));

        // Give the Tanager time to capture a frame.
        Sleep(17);

        // put video capture logic back in reset
        parent->FpgaWrite(0x20, std::vector<byte>({1}));

        // query resolution
        auto timing = parent->GetVideoTiming();

        // compute size of buffer
        uint32_t bufferSizeInDWords = timing.hActive * timing.vActive; // For now, assume good sync and 4 bytes per pixel

        // FX3 requires the read size to be a multiple of 1024 DWORDs
        if(bufferSizeInDWords % 1024)
        {
            bufferSizeInDWords += 1024 - (bufferSizeInDWords % 1024);
        }

        // specify number of dwords to read
        // This is bufferSizeInDWords but in big-endian order in a vector<byte>
        parent->FpgaWrite(
            0x15,
            std::vector<byte>(
                {(uint8_t)((bufferSizeInDWords >> 24) & 0xff),
                 (uint8_t)((bufferSizeInDWords >> 16) & 0xff),
                 (uint8_t)((bufferSizeInDWords >> 8) & 0xff),
                 (uint8_t)(bufferSizeInDWords & 0xff)}));

        // prepare for reading
        parent->FpgaWrite(0x10, std::vector<byte>({3}));

        // initiate read sequencer
        parent->FpgaWrite(0x10, std::vector<byte>({2}));

        // read frame
        auto frameData = parent->ReadEndPointData(bufferSizeInDWords * 4);

        // turn off read sequencer
        parent->FpgaWrite(0x10, std::vector<byte>({3}));

        // Add any extended properties that aren't directly exposable in the IDisplayCapture* interfaces yet
        auto extendedProps = winrt::multi_threaded_observable_map<winrt::hstring, winrt::IInspectable>();
        extendedProps.Insert(L"pixelClock",  winrt::box_value(timing.pixelClock));
        extendedProps.Insert(L"hTotal",      winrt::box_value(timing.hTotal));
        extendedProps.Insert(L"hFrontPorch", winrt::box_value(timing.hFrontPorch));
        extendedProps.Insert(L"hSyncWidth",  winrt::box_value(timing.hSyncWidth));
        extendedProps.Insert(L"hBackPorch",  winrt::box_value(timing.hBackPorch));
        extendedProps.Insert(L"vTotal",      winrt::box_value(timing.vTotal));
        extendedProps.Insert(L"vFrontPorch", winrt::box_value(timing.vFrontPorch));
        extendedProps.Insert(L"vSyncWidth",  winrt::box_value(timing.vSyncWidth));
        extendedProps.Insert(L"vBackPorch",  winrt::box_value(timing.vBackPorch));
        
        auto resolution = winrt::Windows::Graphics::SizeInt32();
        resolution = { timing.hActive, timing.vActive };

        return winrt::make<TanagerDisplayCapture>(frameData, resolution, extendedProps, m_logger);
    }

	void TanagerDisplayInput::FinalizeDisplayState()
    {
        if (auto parent = m_parent.lock())
        {
            parent->FpgaWrite(0x4, std::vector<byte>({0x34})); // HPD high
        }
        else
        {
            m_logger.LogAssert(L"Cannot obtain reference to Tanager object.");
        }

        Sleep(5000);
    }

    void TanagerDisplayInput::SetEdid(std::vector<byte> edid)
    {
        // TODO: Add some sort of size check?
        unsigned short writeAddress;
        switch (m_port)
        {
            case TanagerDisplayInputPort::hdmi:
                writeAddress = 0x400;
                break;
            default:
            case TanagerDisplayInputPort::displayPort:
                m_logger.LogAssert(L"DisplayPort input of Tanager does not currently support setting EDIDs.");
                return;
        }

        if (auto parent = m_parent.lock())
        {
            parent->FpgaWrite(writeAddress, edid);
        }
        else
        {
            m_logger.LogAssert(L"Cannot obtain reference to Tanager object.");
        }
    }


	bool TanagerCaptureCapabilities::CanReturnRawFramesToHost()
    {
        return true;
    }

    bool TanagerCaptureCapabilities::CanReturnFramesToHost()
    {
        return true;
    }

    bool TanagerCaptureCapabilities::CanCaptureFrameSeries()
    {
        return false;
    }

    bool TanagerCaptureCapabilities::CanHotPlug()
    {
        return true;
    }

    bool TanagerCaptureCapabilities::CanConfigureEDID()
    {
        switch (m_port)
		{
		case TanagerDisplayInputPort::hdmi:
			return true;
		case TanagerDisplayInputPort::displayPort:
			return false;
        }

        return false;
    }

    bool TanagerCaptureCapabilities::CanConfigureDisplayID()
    {
        return false;
    }

    uint32_t TanagerCaptureCapabilities::GetMaxDescriptorSize()
    {
        switch (m_port)
		{
		case TanagerDisplayInputPort::hdmi:
			return 1024;
		case TanagerDisplayInputPort::displayPort:
        default:
			throw winrt::hresult_not_implemented();
		}
    }

    TanagerDisplayCapture::TanagerDisplayCapture(
        std::vector<byte> rawCaptureData,
        winrt::Windows::Graphics::SizeInt32 resolution,
        winrt::IMap<winrt::hstring, winrt::IInspectable> extendedProps,
        winrt::ILogger const& logger) :
        m_extendedProps(extendedProps), m_logger(logger)
    {
        if (rawCaptureData.size() == 0)
        {
            throw hresult_invalid_argument();
        }

        typedef struct
        {
            uint64_t pad1 : 2;
            uint64_t red1 : 8;
            uint64_t pad2 : 2;
            uint64_t green1 : 8;
            uint64_t pad3 : 2;
            uint64_t blue1 : 8;
            uint64_t pad4 : 2;
            uint64_t red2 : 8;
            uint64_t pad5 : 2;
            uint64_t green2 : 8;
            uint64_t pad6 : 2;
            uint64_t blue2 : 8;
            uint64_t rsvd : 4;
        } rgbDataType;

        // Yes this is doing a double copy (triple to remove padding) at the moment - because the interfaces are dumb
        // I want to pull the comparisons entirely away from using SoftwareBitmap as the solution here

        std::vector<byte> pixels;
        rgbDataType* rgbData = (rgbDataType*)rawCaptureData.data();
        for (int i = 0; i < rawCaptureData.size() / sizeof(rgbDataType); i++)
        {
            pixels.push_back(rgbData->red1);
            pixels.push_back(rgbData->green1);
            pixels.push_back(rgbData->blue1);
            pixels.push_back(0); // alpha
            pixels.push_back(rgbData->red2);
            pixels.push_back(rgbData->green2);
            pixels.push_back(rgbData->blue2);
            pixels.push_back(0); // alpha
            rgbData++;
        }

        m_bitmap = winrt::SoftwareBitmap(
            winrt::BitmapPixelFormat::Rgba8, resolution.Width, resolution.Height, winrt::BitmapAlphaMode::Ignore);

        auto buff = m_bitmap.LockBuffer(winrt::BitmapBufferAccessMode::Write);
        auto ref = buff.CreateReference();

        m_bitmapDesc = buff.GetPlaneDescription(0);

        // Because reads need to be in chunks of 4096 bytes, pixels can be up to 4096 bytes larger
        if (ref.Capacity() < (pixels.size() - 4096))
        {
            throw hresult_invalid_argument();
        }

        RtlCopyMemory(ref.data(), pixels.data(), ref.Capacity());
    }

    bool TanagerDisplayCapture::CompareCaptureToPrediction(winrt::hstring name, winrt::MicrosoftDisplayCaptureTools::Display::IDisplayEnginePrediction prediction)
    {
        auto predictedBitmap = prediction.GetBitmap();
        auto captureBuffer = m_bitmap.LockBuffer(BitmapBufferAccessMode::Read).CreateReference();
        auto predictBuffer = predictedBitmap.LockBuffer(BitmapBufferAccessMode::Read).CreateReference();

        //
        // Compare the two images. In some capture cards this can be done on the capture device itself. In this generic
        // plugin only RGB8 is supported.
        //
        // TODO: this needs to handle multiple formats
        //

        if (captureBuffer.Capacity() != predictBuffer.Capacity())
        {
            m_logger.LogError(
                winrt::hstring(L"Capture Sizes don't match!  Captured=") + std::to_wstring(captureBuffer.Capacity()) +
                L", Predicted=" + std::to_wstring(predictBuffer.Capacity()));
        }
        else if (0 != memcmp(captureBuffer.data(), predictBuffer.data(), captureBuffer.Capacity()))
        {
            struct PixelStruct
            {
                uint8_t r, g, b, a;
            };

            auto differenceCount = 0;

            PixelStruct* cap = reinterpret_cast<PixelStruct*>(captureBuffer.data());
            PixelStruct* pre = reinterpret_cast<PixelStruct*>(predictBuffer.data());

            // Comparing pixel for pixel takes a very long time at the moment - so let's compare stochastically
            const int samples = 10000;
            const int pixelCount = m_bitmapDesc.Width * m_bitmapDesc.Height;
            for (auto i = 0; i < samples; i++)
            {
                auto index = rand() % pixelCount;

                if (cap[index].r != pre[index].r ||
                    cap[index].g != pre[index].g ||
                    cap[index].b != pre[index].b)
                {
                    differenceCount++;
                }
            }

            {
                auto filename = name + L"_Captured.bmp";
                auto folder = winrt::KnownFolders::PicturesLibrary();
                auto file = folder.CreateFileAsync(filename, winrt::CreationCollisionOption::GenerateUniqueName).get();
                auto stream = file.OpenAsync(winrt::FileAccessMode::ReadWrite).get();
                auto encoder = winrt::BitmapEncoder::CreateAsync(winrt::BitmapEncoder::BmpEncoderId(), stream).get();
                encoder.SetSoftwareBitmap(m_bitmap);

                encoder.FlushAsync().get();
            }
            {
                auto filename = name + L"_Predicted.bmp";
                auto folder = winrt::KnownFolders::PicturesLibrary();
                auto file = folder.CreateFileAsync(filename, winrt::CreationCollisionOption::GenerateUniqueName).get();
                auto stream = file.OpenAsync(winrt::FileAccessMode::ReadWrite).get();
                auto encoder = winrt::BitmapEncoder::CreateAsync(winrt::BitmapEncoder::BmpEncoderId(), stream).get();
                encoder.SetSoftwareBitmap(predictedBitmap);

                encoder.FlushAsync().get();
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

    winrt::Windows::Graphics::SizeInt32 TanagerDisplayCapture::Resolution()
    {
        return {m_bitmapDesc.Width, m_bitmapDesc.Height};
    }

    uint32_t TanagerDisplayCapture::Stride()
    {
        return m_bitmapDesc.Stride;
    }

    winrt::Windows::Graphics::DirectX::DirectXPixelFormat TanagerDisplayCapture::PixelFormat()
    {
        // The BitmapPixelFormat enum type is intentionally compatible with DirectXPixelFormats
        return static_cast<winrt::Windows::Graphics::DirectX::DirectXPixelFormat>(m_bitmap.BitmapPixelFormat());
    }

    winrt::Windows::Foundation::IMemoryBufferReference TanagerDisplayCapture::GetRawPixelData()
    {
        auto buffer = m_bitmap.LockBuffer(winrt::BitmapBufferAccessMode::Read);
        return buffer.CreateReference();
    }

    winrt::Windows::Foundation::Collections::IMapView<winrt::hstring, winrt::Windows::Foundation::IInspectable> TanagerDisplayCapture::ExtendedProperties()
    {
        return m_extendedProps.GetView();
    }

} // namespace winrt::TanagerPlugin::implementation
