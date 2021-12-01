#pragma once
#include "Controller.g.h"
#include "pch.h"
#include "I2cDriver.h"
#include "Fx3FpgaInterface.h"

namespace winrt::TanagerPlugin::implementation
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

    struct Controller : ControllerT<Controller>
    {
        Controller();

        hstring Name();
        com_array<MicrosoftDisplayCaptureTools::CaptureCard::IDisplayInput> EnumerateDisplayInputs();
        void SetConfigData(Windows::Data::Json::IJsonValue data);

       
    private:
        void DiscoverCaptureBoards();
        std::vector<MicrosoftDisplayCaptureTools::CaptureCard::IDisplayInput> m_displayInputs;
        std::vector<std::shared_ptr<IMicrosoftCaptureBoard>> m_captureBoards;
    };
}

namespace winrt::TanagerPlugin::factory_implementation
{
    struct Controller : ControllerT<Controller, implementation::Controller>
    {
    };
}
