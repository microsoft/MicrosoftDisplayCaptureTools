#pragma once
#include "pch.h"
//
// FX3 vendor commands
//
#define VR_VERSION              0x01
#define VR_UART_DATA_TRANSFER   0x10
#define VR_RESET_FPGA           0x11
#define VR_I2C_DATA_TRANSFER    0x12
#define VR_XFER_RESET           0x20
#define VR_XFER_RNW             0x21
#define VR_RECONFIGURE          0x22
#define VR_FLASH_TRANSFER       0xC2
#define VR_DEVICE_RESET         0xC6

class I2cDriver
{
public:
    I2cDriver(winrt::Windows::Devices::Usb::UsbDevice usbDevice);

    uint16_t readRegister(uint16_t address, uint8_t reg, size_t count, uint8_t* buffer);
    char readRegisterByte(uint16_t address, uint8_t reg);

    void writeRegister(uint16_t address, uint8_t reg, size_t count, const uint8_t* buffer);
    void writeRegisterByte(uint16_t address, uint8_t reg, uint8_t value);
    void writeRegisterByteMasked(uint16_t address, uint8_t reg, uint8_t value, uint8_t mask);

private:
    winrt::Windows::Devices::Usb::UsbDevice usbDevice;
};
