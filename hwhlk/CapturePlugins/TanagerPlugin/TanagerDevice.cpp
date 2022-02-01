#include "pch.h"

namespace winrt 
{
    using namespace winrt::Windows::Devices::Enumeration;
    using namespace winrt::Windows::Devices::Usb;
    using namespace winrt::Windows::Graphics::Imaging;
    using namespace winrt::Windows::Devices::Display;
    using namespace winrt::Windows::Devices::Display::Core;
    using namespace winrt::Windows::Storage;
    using namespace winrt::Windows::Storage::Streams;
}
using namespace IteIt68051Plugin;

namespace winrt::TanagerPlugin::implementation
{
const unsigned char it68051i2cAddress = 0x48;

TanagerDevice::TanagerDevice(winrt::param::hstring deviceId) :
    m_usbDevice(nullptr),
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
	}

	TanagerDevice::~TanagerDevice()
	{
	}

	std::vector<MicrosoftDisplayCaptureTools::CaptureCard::IDisplayInput> TanagerDevice::EnumerateDisplayInputs()
	{
		return std::vector<MicrosoftDisplayCaptureTools::CaptureCard::IDisplayInput>
		{
			winrt::make<TanagerDisplayInput>(this->weak_from_this(), TanagerDisplayInputPort::hdmi),
			winrt::make<TanagerDisplayInput>(this->weak_from_this(), TanagerDisplayInputPort::displayPort),
		};
	}

	void TanagerDevice::TriggerHdmiCapture()
	{
		throw winrt::hresult_not_implemented();
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

	void TanagerDevice::FlashFpgaFirmware(Windows::Foundation::Uri uri)
	{
		m_fpga.FlashFpgaFirmware(uri);
	}

	void TanagerDevice::FlashFx3Firmware(Windows::Foundation::Uri uri)
	{
		m_fpga.FlashFx3Firmware(uri);
	}

	FirmwareVersionInfo TanagerDevice::GetFirmwareVersionInfo()
	{
		return m_fpga.GetFirmwareVersionInfo();
	}

    IteIt68051Plugin::VideoTiming TanagerDevice::getVideoTiming()
    {
        return hdmiChip.GetVideoTiming();
    }

	TanagerDisplayInput::TanagerDisplayInput(std::weak_ptr<TanagerDevice> parent, TanagerDisplayInputPort port)
		: m_parent(parent),
		  m_port(port)
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
		}

        throw winrt::hresult_invalid_argument();
    }

    void TanagerDisplayInput::SetDescriptor(MicrosoftDisplayCaptureTools::Display::IMonitorDescriptor descriptor)
    {
        if (descriptor.GetType() != MicrosoftDisplayCaptureTools::Display::MonitorDescriptorType::EDID)
        {
            throw winrt::hresult_invalid_argument();
        }

        auto edidMemoryReference = descriptor.GetData();
        auto edidBuffer = edidMemoryReference.data();
        std::vector<byte> edid(edidBuffer, edidBuffer + edidMemoryReference.Capacity());
        
        SetEdid(edid);
    }

    MicrosoftDisplayCaptureTools::CaptureCard::ICaptureTrigger TanagerDisplayInput::GetCaptureTrigger()
    {
        return MicrosoftDisplayCaptureTools::CaptureCard::ICaptureTrigger();
    }

	MicrosoftDisplayCaptureTools::CaptureCard::ICaptureCapabilities TanagerDisplayInput::GetCapabilities()
	{
        auto caps = winrt::make<TanagerCaptureCapabilities>();
        return caps;
    }

    MicrosoftDisplayCaptureTools::CaptureCard::IDisplayCapture TanagerDisplayInput::CaptureFrame()
    {
        auto parent = m_parent.lock();
        if (!parent)
        {
            throw winrt::hresult_error();
        }

        // Capture frame in DRAM
        parent->FpgaWrite(0x20, std::vector<byte>({0}));

        // Give the Tanager time to capture a frame.
        Sleep(17);

        // put video capture logic back in reset
        parent->FpgaWrite(0x20, std::vector<byte>({1}));
        // query resolution
        auto timing = parent->getVideoTiming();
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

        return winrt::make<TanagerDisplayCapture>(frameData);
    }

	void TanagerDisplayInput::FinalizeDisplayState()
    {
        SetEdid({0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x10, 0xac, 0x84, 0x41, 0x4c, 0x34, 0x45, 0x42, 0x1e, 0x1e, 0x01,
                 0x04, 0xa5, 0x3c, 0x22, 0x78, 0x3a, 0x48, 0x15, 0xa7, 0x56, 0x52, 0x9c, 0x27, 0x0f, 0x50, 0x54, 0xa5, 0x4b, 0x00,
                 0x71, 0x4f, 0x81, 0x80, 0xa9, 0xc0, 0xd1, 0xc0, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x3a, 0x80,
                 0x18, 0x71, 0x38, 0x2d, 0x40, 0x58, 0x2c, 0x45, 0x00, 0x56, 0x50, 0x21, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0xff,
                 0x00, 0x36, 0x56, 0x54, 0x48, 0x5a, 0x31, 0x33, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xfc, 0x00,
                 0x44, 0x45, 0x4c, 0x4c, 0x20, 0x50, 0x32, 0x37, 0x31, 0x39, 0x48, 0x0a, 0x20, 0x00, 0x00, 0x00, 0xfd, 0x00, 0x38,
                 0x4c, 0x1e, 0x53, 0x11, 0x01, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0xec});

        if (auto parent = m_parent.lock())
        {
            parent->FpgaWrite(0x4, std::vector<byte>({0x34})); // HPD high
        }
        else
        {
            throw winrt::hresult_error();
        }

        Sleep(5000);
    }

    void TanagerDisplayInput::SetEdid(std::vector<byte> edid)
    {
        // TODO: Add some sort of size check?

        if (auto parent = m_parent.lock())
        {
            parent->FpgaWrite(0x400, edid);
        }
        else
        {
            throw winrt::hresult_error();
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
        return false;
    }

    bool TanagerCaptureCapabilities::CanConfigureEDID()
    {
        return false;
    }

    bool TanagerCaptureCapabilities::CanConfigureDisplayID()
    {
        return false;
    }

    uint32_t TanagerCaptureCapabilities::GetMaxDescriptorSize()
    {
        throw hresult_not_implemented();
    }

    TanagerDisplayCapture::TanagerDisplayCapture(std::vector<byte> rawCaptureData)
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
            pixels.push_back(rgbData->blue1);
            pixels.push_back(rgbData->green1);
            pixels.push_back(rgbData->red1);
            pixels.push_back(0); // alpha
            pixels.push_back(rgbData->blue2);
            pixels.push_back(rgbData->green2);
            pixels.push_back(rgbData->red2);
            pixels.push_back(0); // alpha
            rgbData++;
        }

        m_bitmap = winrt::SoftwareBitmap(
            winrt::BitmapPixelFormat::Rgba8, 1920, 1080, winrt::BitmapAlphaMode::Ignore);

        auto buff = m_bitmap.LockBuffer(winrt::BitmapBufferAccessMode::Write);
        auto ref = buff.CreateReference();

        if (ref.Capacity() < pixels.size())
        {
            throw hresult_invalid_argument();
        }

        RtlCopyMemory(ref.data(), pixels.data(), pixels.size());
    }

    void TanagerDisplayCapture::CompareCaptureToPrediction(winrt::hstring name, winrt::MicrosoftDisplayCaptureTools::Display::IDisplayEnginePrediction prediction)
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

        if (captureBuffer.Capacity() != predictBuffer.Capacity())
        {
            printf("Capture Sizes don't match!  Captured=%d, Predicted=%d\n\n",
                captureBuffer.Capacity(),
                predictBuffer.Capacity());
        }
        else if (0 != memcmp(captureBuffer.data(), predictBuffer.data(), captureBuffer.Capacity()))
        {
            {
                struct PixelStruct
                {
                    uint8_t r, g, b, a;
                };

                PixelStruct* cap = reinterpret_cast<PixelStruct*>(captureBuffer.data());
                PixelStruct* pre = reinterpret_cast<PixelStruct*>(predictBuffer.data());
                for (auto i = 0; i < captureBuffer.Capacity(); i++)
                {
                    if (cap->r != pre->r ||
                        cap->g != pre->g ||
                        cap->b != pre->b)
                    {
                        printf(
                            "First Difference: (%d,%d)\n\tCaptured  - %d, %d, %d\n\tPredicted - %d, %d, %d\n\n",
                            i%1920,
                            i/1920,
                            cap->r,
                            cap->g,
                            cap->b,
                            pre->r,
                            pre->g,
                            pre->b);

                        break;
                    }

                    cap++;
                    pre++;
                }
            }

            {
                auto folder = winrt::KnownFolders::PicturesLibrary();
                auto file = folder.CreateFileAsync(L"Captured.bmp", winrt::CreationCollisionOption::GenerateUniqueName).get();
                auto stream = file.OpenAsync(winrt::FileAccessMode::ReadWrite).get();
                auto encoder = winrt::BitmapEncoder::CreateAsync(winrt::BitmapEncoder::BmpEncoderId(), stream).get();
                encoder.SetSoftwareBitmap(m_bitmap);

                encoder.FlushAsync().get();
            }
            {
                auto folder = winrt::KnownFolders::PicturesLibrary();
                auto file = folder.CreateFileAsync(L"Predicted.bmp", winrt::CreationCollisionOption::GenerateUniqueName).get();
                auto stream = file.OpenAsync(winrt::FileAccessMode::ReadWrite).get();
                auto encoder = winrt::BitmapEncoder::CreateAsync(winrt::BitmapEncoder::BmpEncoderId(), stream).get();
                encoder.SetSoftwareBitmap(prediction.GetBitmap());

                encoder.FlushAsync().get();
            }
        }
    }

    winrt::Windows::Foundation::IMemoryBufferReference TanagerDisplayCapture::GetRawPixelData()
    {
        auto buffer = m_bitmap.LockBuffer(winrt::BitmapBufferAccessMode::Read);
        return buffer.CreateReference();
    }

} // namespace winrt::TanagerPlugin::implementation
