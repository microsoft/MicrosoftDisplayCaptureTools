#pragma once
#include "pch.h"
#include <vector>

namespace winrt::TestPlugin::implementation
{
    class Fx3FpgaInterface
    {
    public:
        Fx3FpgaInterface();
        void SetUsbDevice(winrt::Windows::Devices::Usb::UsbDevice usbDevice);
        void Write(unsigned short address, std::vector<byte> data);
        std::vector<byte> Read(unsigned short address, UINT16 size);
        std::vector<byte> ReadEndPointData(UINT32 dataSize);

    private:
        DWORD ReadSetupPacket(winrt::Windows::Storage::Streams::Buffer readBuffer, UINT16 address, UINT16 len, ULONG* bytesRead);
        winrt::Windows::Devices::Usb::UsbDevice m_usbDevice;
    };
}