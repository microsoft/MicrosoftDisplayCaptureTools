#include "pch.h"
#include <filesystem>

namespace winrt 
{
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Windows::Foundation::Collections;
    using namespace winrt::Windows::Devices::Enumeration;
    using namespace winrt::Windows::Devices::Usb;
    using namespace winrt::Windows::Graphics::Imaging;
    using namespace winrt::Windows::Graphics::DirectX;
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

TanagerDevice::TanagerDevice() :
    m_usbDevice(nullptr),
    m_deviceId(deviceId),
    hdmiChip(
        [&](uint8_t address, uint8_t value) // I2C write
        { m_pDriver->writeRegisterByte(it68051i2cAddress, address, value); },
        [&](uint8_t address) // I2C read
        { return m_pDriver->readRegisterByte(it68051i2cAddress, address); })
	{
		m_usbDevice = UsbDevice::FromIdAsync(deviceId).get();

		if (!m_usbDevice)
		{
			throw_hresult(E_FAIL);
		}

        // Attempt to reboot the Tanager device
        /* {
            auto rebootPacket = winrt::UsbSetupPacket();
            auto rebootPacketRequestType = winrt::UsbControlRequestType();
            rebootPacketRequestType.AsByte(0x40);
            rebootPacket.RequestType(rebootPacketRequestType);
            rebootPacket.Request(0xc6);

            auto ret = m_usbDevice.SendControlOutTransferAsync(rebootPacket).get();

            if (0 == ret)
            {
                // If the command to start a board reset succeeded - Sleep for a few seconds to let the board come back up.
                Sleep(10000);
            }
        }*/

		m_fpga.SetUsbDevice(m_usbDevice);
		m_pDriver = std::make_shared<I2cDriver>(m_usbDevice);
        m_fpga.SysReset(); // Blocks until FPGA is ready
		hdmiChip.Initialize();

        Logger().LogNote(L"Initializing Tanager Device: " + m_deviceId);
	}

	TanagerDevice::~TanagerDevice()
    {
	}

	std::vector<MicrosoftDisplayCaptureTools::CaptureCard::IDisplayInput> TanagerDevice::EnumerateDisplayInputs()
	{
		return std::vector<MicrosoftDisplayCaptureTools::CaptureCard::IDisplayInput>
		{
			winrt::make<TanagerDisplayInputHdmi>(this->weak_from_this()),
			winrt::make<TanagerDisplayInputDisplayPort>(this->weak_from_this())
		};
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

    void TanagerDevice::SelectDisplayPortEDID(USHORT value)
    {
        m_fpga.SelectDisplayPortEDID(value);
    }

    bool TanagerDevice::IsVideoLocked()
    {
        return hdmiChip.IsVideoLocked();
    }

    void TanagerDevice::I2cWriteData(uint16_t i2cAddress, uint8_t address, std::vector<byte> data)
    {
        // Send this down in chunks
        const size_t writeBlockSize = 0x20;
        for (size_t i = 0, remaining = data.size(); remaining > 0; i += writeBlockSize, remaining -= min(writeBlockSize, remaining))
        {
            size_t amountToWrite = min(writeBlockSize, remaining);
            m_pDriver->writeRegister(i2cAddress, address + i, static_cast<uint32_t>(amountToWrite), data.data() + i);
            Sleep(100);
        }
    }

    std::mutex& TanagerDevice::SelectHdmi()
    {
        hdmiChip.SelectHdmi();
        return m_changingPortsLocked;
    }

    std::mutex& TanagerDevice::SelectDisplayPort()
    {
        hdmiChip.SelectDisplayPort();
        return m_changingPortsLocked;
    }

    IteIt68051Plugin::VideoTiming TanagerDevice::GetVideoTiming()
    {
        return hdmiChip.GetVideoTiming();
    }

    IteIt68051Plugin::aviInfoframe TanagerDevice::GetAviInfoframe()
    {
        return hdmiChip.GetAviInfoframe();
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

    TanagerDisplayCapture::TanagerDisplayCapture(
        std::vector<byte> rawCaptureData,
        winrt::Windows::Graphics::SizeInt32 resolution,
        winrt::IMap<winrt::hstring, winrt::IInspectable> extendedProps) :
        m_extendedProps(extendedProps)
    {
        if (rawCaptureData.size() == 0)
        {
            throw hresult_invalid_argument();
        }

        m_frameData = FrameData();

        // TODO: isolate this into a header supporting different masks
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

        auto pixelDataWriter = DataWriter();
        rgbDataType* rgbData = (rgbDataType*)rawCaptureData.data();
        while ((void*)rgbData < (void*)(rawCaptureData.data() + rawCaptureData.size()))
        {
            pixelDataWriter.WriteByte(rgbData->red1);
            pixelDataWriter.WriteByte(rgbData->green1);
            pixelDataWriter.WriteByte(rgbData->blue1);
            pixelDataWriter.WriteByte(0xFF); // Alpha padding
            pixelDataWriter.WriteByte(rgbData->red2);
            pixelDataWriter.WriteByte(rgbData->green2);
            pixelDataWriter.WriteByte(rgbData->blue2);
            pixelDataWriter.WriteByte(0xFF); // Alpha padding
            rgbData++;
        }

        m_frameData.Data(pixelDataWriter.DetachBuffer());
        m_frameData.Resolution(resolution);

        // This is only supporting 8bpc RGB444 currently
        FrameFormatDescription desc{0};
        desc.BitsPerPixel = 24;
        desc.Stride = resolution.Width * 3; // There is no padding with this capture
        desc.PixelFormat = DirectXPixelFormat::Unknown; // Specify that we don't have an exact match to the input DirectX formats
        desc.PixelEncoding = DisplayWireFormatPixelEncoding::Rgb444;
        desc.Eotf = DisplayWireFormatEotf::Sdr;

        m_frameData.FormatDescription(desc);
    }

    bool TanagerDisplayCapture::CompareCaptureToPrediction(winrt::hstring name, winrt::MicrosoftDisplayCaptureTools::ConfigurationTools::IPredictionData prediction)
    {
        auto predictedFrameData = prediction.FrameData();

        if (predictedFrameData.Resolution().Height != m_frameData.Resolution().Height || 
            predictedFrameData.Resolution().Width != m_frameData.Resolution().Width)
        {
            Logger().LogError(winrt::hstring(L"Predicted resolution (") + 
                to_hstring(predictedFrameData.Resolution().Width) + L"," +
                to_hstring(predictedFrameData.Resolution().Height) + L"), Captured Resolution(" + 
                to_hstring(m_frameData.Resolution().Width) + L"," +
                to_hstring(m_frameData.Resolution().Height) + L")");
        }

        auto captureBuffer = m_frameData.Data();
        auto predictBuffer = predictedFrameData.Data();

        if (captureBuffer.Length() < predictBuffer.Length())
        {
            Logger().LogError(
                winrt::hstring(L"Capture should be at least as large as prediction") + std::to_wstring(captureBuffer.Length()) +
                L", Predicted=" + std::to_wstring(predictBuffer.Length()));
        }
        else if (0 == memcmp(captureBuffer.data(), predictBuffer.data(), predictBuffer.Length()))
        {
            Logger().LogNote(L"Capture and Prediction perfectly match!");
        }
        else
        {
            Logger().LogWarning(L"Capture did not exactly match prediction! Attempting comparison with tolerance.");

            {
                auto filename = name + L"_Captured";
                auto folder = winrt::StorageFolder::GetFolderFromPathAsync(std::filesystem::current_path().c_str()).get();

                // Attempt to extract a renderable approximation of the frame
                auto bitmap = m_frameData.GetRenderableApproximationAsync().get();
                filename = filename + (bitmap ? L".png" : L".bin");

                auto file = folder.CreateFileAsync(filename, winrt::CreationCollisionOption::ReplaceExisting).get();
                auto stream = file.OpenAsync(winrt::FileAccessMode::ReadWrite).get();

                if (bitmap)
                {
                    // We could extract a valid bitmap from the data, save that way
                    auto encoder = BitmapEncoder::CreateAsync(BitmapEncoder::PngEncoderId(), stream).get();
                    encoder.SetSoftwareBitmap(bitmap);
                }
                else
                {
                    // We could not extract a valid bitmap from the data, just save the binary to disk
                    stream.WriteAsync(captureBuffer).get();
                }

                stream.FlushAsync().get();
                stream.Close();

                Logger().LogNote(L"Saving captured data here: " + filename);
            }
            {
                auto filename = name + L"_Predicted";
                auto folder = winrt::StorageFolder::GetFolderFromPathAsync(std::filesystem::current_path().c_str()).get();

                // Attempt to extract a renderable approximation of the frame
                SoftwareBitmap bitmap{nullptr};
                auto predictedFrameDataComparisons = predictedFrameData.as<IFrameDataComparisons>();
                if (predictedFrameDataComparisons && (bitmap = predictedFrameDataComparisons.GetRenderableApproximationAsync().get()))
                {
                    filename = filename + L".png";
                }
                else 
                {
                    filename = filename + L".bin";
                }

                auto file = folder.CreateFileAsync(filename, winrt::CreationCollisionOption::ReplaceExisting).get();
                auto stream = file.OpenAsync(winrt::FileAccessMode::ReadWrite).get();

                if (bitmap)
                {
                    // We could extract a valid bitmap from the data, save that way
                    auto encoder = BitmapEncoder::CreateAsync(BitmapEncoder::PngEncoderId(), stream).get();
                    encoder.SetSoftwareBitmap(bitmap);
                }
                else
                {
                    // We could not extract a valid bitmap from the data, just save the binary to disk
                    stream.WriteAsync(predictBuffer).get();
                }

                stream.FlushAsync().get();
                stream.Close();

                Logger().LogNote(L"Saving predicted data here: " + filename);
            }

            struct PixelStruct
            {
                uint8_t r, g, b, a;
            };

            auto differenceCount = 0;

            PixelStruct* cap = reinterpret_cast<PixelStruct*>(captureBuffer.data());
            PixelStruct* pre = reinterpret_cast<PixelStruct*>(predictBuffer.data());

            // Comparing pixel for pixel takes a very long time at the moment - so let's compare stochastically
            const int samples = 10000;
            const int pixelCount = m_frameData.Resolution().Width * m_frameData.Resolution().Height;
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

            float diff = (float)differenceCount / (float)samples;
            if (diff > 0.10f)
            {
                std::wstring msg;
                std::format_to(std::back_inserter(msg), "{:.2f}% of sampled pixels did not match!", diff*100);
                Logger().LogError(msg);

                return false;
            }
        }

        return true;
    }

    winrt::MicrosoftDisplayCaptureTools::Framework::IFrameData TanagerDisplayCapture::GetFrameData()
    {
        return m_frameData;
    }

    winrt::Windows::Foundation::Collections::IMapView<winrt::hstring, winrt::Windows::Foundation::IInspectable> TanagerDisplayCapture::ExtendedProperties()
    {
        return m_extendedProps.GetView();
    }

} // namespace winrt::TanagerPlugin::implementation
