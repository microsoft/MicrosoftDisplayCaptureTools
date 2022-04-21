#pragma once
#include "pch.h"
#include "Controller.h"
#include <vector>

namespace winrt::TanagerPlugin::implementation
{
    class Fx3FpgaInterface
    {
    public:
        Fx3FpgaInterface();
        void SetUsbDevice(winrt::Windows::Devices::Usb::UsbDevice usbDevice);
        void Write(unsigned short address, std::vector<byte> data);
        std::vector<byte> Read(unsigned short address, UINT16 size);
        std::vector<byte> ReadEndPointData(UINT32 dataSize);
        void FlashFpgaFirmware(winrt::hstring uri);
        void FlashFx3Firmware(winrt::hstring uri);
        struct FirmwareVersionInfo GetFirmwareVersionInfo();
        void SysReset();
        bool IsFpgaReady();

    private:
        winrt::Windows::Devices::Usb::UsbDevice m_usbDevice;
    };
}