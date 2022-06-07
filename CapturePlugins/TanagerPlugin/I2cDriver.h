#pragma once
#include "pch.h"
//
// FX3 vendor commands
//
#define VR_VERSION 0x01
#define VR_UART_DATA_TRANSFER 0x10
#define VR_RESET_FPGA 0x11
#define VR_I2C_DATA_TRANSFER 0x12
#define VR_FPGA_READY 0x13
#define VR_XFER_RESET 0x20
#define VR_XFER_RNW 0x21
#define VR_RECONFIGURE 0x22
#define VR_SYS_RESET 0x23
#define VR_HDMI_EDID_SELECT 0x24
#define VR_DP_EDID_SELECT 0x25
#define VR_DP2_EDID_SELECT 0x26
#define VR_SPI_EPCQ64_WRITE_BYTES 0x27
#define VR_SPI_EPCQ64_READ_BYTES 0x28
#define VR_SPI_EPCQ64_BULK_ERASE 0x29
#define VR_SPI_EPCQ64_READ_ID 0x2A
#define VR_SPI_EPCQ64_READ_STATUS_REGISTER 0x2B
#define VR_SPI_SST25VF080B_WRITE_BYTES 0x2C
#define VR_SPI_SST25VF080B_READ_BYTES 0x2D
#define VR_SPI_SST25VF080B_BULK_ERASE 0x2E
#define VR_SPI_SST25VF080B_READ_ID 0x2F
#define VR_SPI_EPCQ64_SELECT 0x30
#define VR_SPI_SST25VF080B_SELECT 0x31

class I2cDriver
{
public:
    I2cDriver(winrt::Windows::Devices::Usb::UsbDevice usbDevice);

    uint16_t readRegister(uint16_t address, uint8_t reg, uint16_t count, uint8_t* buffer);
    char readRegisterByte(uint16_t address, uint8_t reg);

    void writeRegister(uint16_t address, uint8_t reg, uint32_t count, const uint8_t* buffer);
    void writeRegisterByte(uint16_t address, uint8_t reg, uint8_t value);
    void writeRegisterByteMasked(uint16_t address, uint8_t reg, uint8_t value, uint8_t mask);

private:
    winrt::Windows::Devices::Usb::UsbDevice usbDevice;
};
