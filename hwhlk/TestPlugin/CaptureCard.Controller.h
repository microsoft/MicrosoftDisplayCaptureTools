#pragma once
#include "CaptureCard.Controller.g.h"
#include "I2cDriver.h"
#include "SampleDisplayCapture.h"

namespace winrt::CaptureCard::implementation
{
    // Provides an abstraction for different boards to provide display inputs
    class IMicrosoftCaptureBoard abstract
    {
    public:
        virtual std::vector<CaptureCard::IDisplayInput> EnumerateDisplayInputs() = 0;
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
    class FrankenboardDevice : public IMicrosoftCaptureBoard
    {
    public:
        FrankenboardDevice(winrt::param::hstring deviceId);
        ~FrankenboardDevice();

        CaptureCard::IDisplayInput GetHdmiInput();
        void TriggerHdmiCapture();
        void FpgaWrite(unsigned short address, std::vector<byte> data);
        std::vector<byte> FpgaRead(unsigned short address, UINT16 dataSize);
        DWORD fpgaReadSetupPacket (winrt::Windows::Storage::Streams::Buffer readBuffer, UINT16 address, UINT16 len, ULONG* bytesRead);

    private:
        void SetEdid(std::vector<byte> Edid);
        void SetHpd(bool isPluggedIn);
        winrt::Windows::Devices::Usb::UsbDevice m_usbDevice;
        CaptureCard::IDisplayInput m_hdmiInput;
        std::shared_ptr<I2cDriver> pDriver;
    };

    // Represents a single real Microsoft capture device. Initializes and keeps device state.
    class MicrosoftRealDevice : IMicrosoftCaptureBoard
    {

    };

    struct Controller : ControllerT<Controller>
    {
        Controller();

        hstring Name();
        com_array<CaptureCard::IDisplayInput> EnumerateDisplayInputs();
        ConfigurationTools::ConfigurationToolbox GetToolbox();

        std::vector<CaptureCard::IDisplayInput> m_displayInputs;
        std::vector<std::shared_ptr<IMicrosoftCaptureBoard>> m_captureBoards;
       

    private:
        void DiscoverCaptureBoards();
        void InitiateCapture(std::shared_ptr<IMicrosoftCaptureBoard> singleCapture);
    };
}



namespace winrt::CaptureCard::factory_implementation
{
    struct Controller : ControllerT<Controller, implementation::Controller>
    {
    };
}
