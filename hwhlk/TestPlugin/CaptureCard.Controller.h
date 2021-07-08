#pragma once
#include "CaptureCard.Controller.g.h"
#include "I2cDriver.h"
#include "Singleton.h"

namespace winrt::CaptureCard::implementation
{

#define TCA6416A_BANK_0 0
#define TCA6416A_BANK_1 1

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
    class FrankenboardDevice
    {
    public:
        FrankenboardDevice(winrt::param::hstring deviceId);
        ~FrankenboardDevice();
        CaptureCard::IDisplayInput GetHdmiInput();
        void TriggerHdmiCapture();
        //Buffer FpgaRead(unsigned short address, std::vector<byte> data);
    private:
        void FpgaWrite(unsigned short address, std::vector<byte> data);
        Buffer FpgaRead(unsigned short address, std::vector<byte> data);
        DWORD FpgaRead(winrt::Windows::Storage::Streams::Buffer readBuffer, UINT16 address, UINT16 len, ULONG* bytesRead);
        void SetEdid(std::vector<byte> Edid);
        void SetHpd(bool isPluggedIn);
        winrt::Windows::Devices::Usb::UsbDevice m_usbDevice;
        CaptureCard::IDisplayInput m_hdmiInput;
        std::shared_ptr<I2cDriver> pDriver;
    };

    struct Controller : ControllerT<Controller>,
        public std::enable_shared_from_this<Controller>,
        public Singleton<Controller>
    {
    
        Controller();

        hstring Name();
        com_array<CaptureCard::IDisplayInput> EnumerateDisplayInputs();
        ConfigurationTools::ConfigurationToolbox GetToolbox();

        std::vector<CaptureCard::IDisplayInput> m_displayInputs;
        std::vector<std::shared_ptr<FrankenboardDevice>> m_frankenboardDevices;
        
        //Instantiating pointer to self & singleton
        std::weak_ptr<winrt::CaptureCard::implementation::Controller> Ctrl_ptr;
        winrt::Windows::Storage::Streams::Buffer FpgaRead()
        {
            return Ctrl_ptr.FpgaRead();
        }
    };
}



namespace winrt::CaptureCard::factory_implementation
{
    struct Controller : ControllerT<Controller, implementation::Controller>
    {
    };
}