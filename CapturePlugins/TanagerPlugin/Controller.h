#pragma once
#include "Controller.g.h"
#include "ControllerFactory.g.h"
#include "pch.h"
#include "I2cDriver.h"
#include "Fx3FpgaInterface.h"

namespace winrt::TanagerPlugin::implementation
{
    struct FirmwareVersionInfo
    {
        uint8_t fx3FirmwareVersionMajor;
        uint8_t fx3FirmwareVersionMinor;
        uint8_t fx3FirmwareVersionPatch;
        uint8_t fpgaFirmwareVersionMajor;
        uint8_t fpgaFirmwareVersionMinor;
        uint8_t fpgaFirmwareVersionPatch;
        uint8_t hardwareRevision;

        std::tuple<uint8_t, uint8_t, uint8_t> GetFx3FirmwareVersion()
        {
            return {fx3FirmwareVersionMajor, fx3FirmwareVersionMinor, fx3FirmwareVersionPatch};
        }
        std::tuple<uint8_t, uint8_t, uint8_t> GetFpgaFirmwareVersion()
        {
            return {fpgaFirmwareVersionMajor, fpgaFirmwareVersionMinor, fpgaFirmwareVersionPatch};
        }
    };

    // Provides an abstraction for different boards to provide display inputs
    class IMicrosoftCaptureBoard abstract
    {
    public:
        virtual winrt::hstring GetDeviceId() = 0;
        virtual std::vector<MicrosoftDisplayCaptureTools::CaptureCard::IDisplayInput> EnumerateDisplayInputs() = 0;

        virtual void TriggerHdmiCapture() = 0;
        virtual std::vector<byte> ReadEndPointData(UINT32 dataSize) = 0;
        virtual std::vector<byte> FpgaRead(unsigned short address, UINT16 dataSize) = 0;
        virtual void FpgaWrite(unsigned short address, std::vector<byte> data) = 0;

        // Firmware update support
        virtual winrt::Windows::Foundation::IAsyncAction UpdateFirmwareAsync() = 0;
        virtual void FlashFpgaFirmware(winrt::hstring filePath) = 0;
        virtual void FlashFx3Firmware(winrt::hstring filePath) = 0;
        virtual FirmwareVersionInfo GetFirmwareVersionInfo() = 0;
        virtual MicrosoftDisplayCaptureTools::CaptureCard::ControllerFirmwareState GetFirmwareState() = 0;
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
        Controller(winrt::MicrosoftDisplayCaptureTools::Framework::ILogger const& logger);

        hstring Name();
        com_array<MicrosoftDisplayCaptureTools::CaptureCard::IDisplayInput> EnumerateDisplayInputs();
        void SetConfigData(Windows::Data::Json::IJsonValue data);

        hstring Version()
        {
            return L"0.1";
        };

        // IControllerWithFirmware
        MicrosoftDisplayCaptureTools::CaptureCard::ControllerFirmwareState FirmwareState();
        Windows::Foundation::IAsyncAction UpdateFirmwareAsync();
        winrt::hstring FirmwareVersion();

    private:
        void DiscoverCaptureBoards();
        std::vector<MicrosoftDisplayCaptureTools::CaptureCard::IDisplayInput> m_displayInputs;
        std::vector<std::shared_ptr<IMicrosoftCaptureBoard>> m_captureBoards;

        const winrt::MicrosoftDisplayCaptureTools::Framework::ILogger m_logger{nullptr};
    };

    struct ControllerFactory : ControllerFactoryT<ControllerFactory>
    {
        ControllerFactory() = default;

        winrt::MicrosoftDisplayCaptureTools::CaptureCard::IController CreateController(winrt::MicrosoftDisplayCaptureTools::Framework::ILogger const& logger);
    };
}

namespace winrt::TanagerPlugin::factory_implementation
{
    struct Controller : ControllerT<Controller, implementation::Controller>
    {
    };
    struct ControllerFactory : ControllerFactoryT<ControllerFactory, implementation::ControllerFactory>
    {
    };
}
