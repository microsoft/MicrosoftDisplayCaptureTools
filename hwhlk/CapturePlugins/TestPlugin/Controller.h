#pragma once
#include "Controller.g.h"
#include "pch.h"
#include "I2cDriver.h"
#include "Fx3FpgaInterface.h"

namespace winrt::TestPlugin::implementation
{
    struct FirmwareVersionInfo
    {
        byte fx3FirmwareVersionMajor;
        byte fx3FirmwareVersionMinor;
        byte fx3FirmwareVersionPatch;
        byte fpgaFirmwareVersionMajor;
        byte fpgaFirmwareVersionMinor;
        byte fpgaFirmwareVersionPatch;
        byte hardwareRevision;
    };

    // Provides an abstraction for different boards to provide display inputs
    class IMicrosoftCaptureBoard abstract
    {
    public:
        virtual std::vector<MicrosoftDisplayCaptureTools::CaptureCard::IDisplayInput> EnumerateDisplayInputs() = 0;

        virtual void TriggerHdmiCapture() = 0;
        virtual std::vector<byte> ReadEndPointData(UINT32 dataSize) = 0;
        virtual std::vector<byte> FpgaRead(unsigned short address, UINT16 dataSize) = 0;
        virtual void FpgaWrite(unsigned short address, std::vector<byte> data) = 0;
        virtual void FlashFpgaFirmware(Windows::Foundation::Uri uri) = 0;
        virtual void FlashFx3Firmware(Windows::Foundation::Uri uri) = 0;
        virtual FirmwareVersionInfo GetFirmwareVersionInfo() = 0;
    };

#define TCA6416A_BANK_0 0
#define TCA6416A_BANK_1 1

    // I2C bus device
    class TiTca6416a
    {
    public:
        TiTca6416a(unsigned char address, std::shared_ptr<I2cDriver> pDriver);
        ~TiTca6416a();

        void SetOutputDirection(char bank, unsigned char value, unsigned char mask);
        void SetOutput(char bank, unsigned char value, unsigned char mask);
        

    private:
        unsigned char address;
        std::shared_ptr<I2cDriver> pDriver;
    };

    // Represents a single Frankenboard USB device. Initializes and keeps device state.
    class FrankenboardDevice : public IMicrosoftCaptureBoard, std::enable_shared_from_this<FrankenboardDevice>
    {
    public:
        FrankenboardDevice(winrt::param::hstring deviceId);
        ~FrankenboardDevice();

        std::vector<MicrosoftDisplayCaptureTools::CaptureCard::IDisplayInput> EnumerateDisplayInputs() override;
        void TriggerHdmiCapture() override;
        void FpgaWrite(unsigned short address, std::vector<byte> data) override;
        std::vector<byte> FpgaRead (unsigned short address, UINT16 size) override;
        std::vector<byte> ReadEndPointData(UINT32 dataSize) override;
        void FlashFpgaFirmware(Windows::Foundation::Uri uri) override;
        void FlashFx3Firmware(Windows::Foundation::Uri uri) override;
        FirmwareVersionInfo GetFirmwareVersionInfo() override;

    private:
        void SetEdid(std::vector<byte> Edid);
        void SetHpd(bool isPluggedIn);
        winrt::Windows::Devices::Usb::UsbDevice m_usbDevice;
        MicrosoftDisplayCaptureTools::CaptureCard::IDisplayInput m_hdmiInput;
        std::shared_ptr<I2cDriver> pDriver;
        Fx3FpgaInterface m_fpga;
    };

    struct Controller : ControllerT<Controller>
    {
        Controller();

        hstring Name();
        com_array<MicrosoftDisplayCaptureTools::CaptureCard::IDisplayInput> EnumerateDisplayInputs();

        MicrosoftDisplayCaptureTools::ConfigurationTools::IConfigurationToolbox GetToolbox();

        std::vector<MicrosoftDisplayCaptureTools::CaptureCard::IDisplayInput> m_displayInputs;
        std::vector<std::shared_ptr<IMicrosoftCaptureBoard>> m_captureBoards;
       

    private:
        void DiscoverCaptureBoards();
        //void InitiateCapture(std::shared_ptr<IMicrosoftCaptureBoard> singleCapture);
    };
}



namespace winrt::TestPlugin::factory_implementation
{
    struct Controller : ControllerT<Controller, implementation::Controller>
    {
    };
}
