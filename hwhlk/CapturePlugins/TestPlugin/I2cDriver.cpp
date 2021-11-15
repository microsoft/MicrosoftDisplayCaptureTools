#include "pch.h"
#include "I2cDriver.h"

using namespace winrt;
using namespace winrt::Windows::Devices::Enumeration;
using namespace winrt::Windows::Devices::Usb;
using namespace winrt::Windows::Storage::Streams;

I2cDriver::I2cDriver(winrt::Windows::Devices::Usb::UsbDevice usbDevice) :
	usbDevice(usbDevice)
{
}

void I2cDriver::writeRegister(uint16_t address, uint8_t reg, uint32_t count, const uint8_t* buffer)
{
	UsbSetupPacket setupPacket;
	UsbControlRequestType requestType;
	requestType.AsByte(0x40); // 0_10_00000
	setupPacket.RequestType(requestType);
	setupPacket.Request(VR_I2C_DATA_TRANSFER);
	setupPacket.Value(address);
	setupPacket.Index(reg);
	setupPacket.Length(count);

	// HACK just make this method use IBuffer eventually
	Buffer writeBuffer(count);
	writeBuffer.Length(count);
	memcpy_s(writeBuffer.data(), writeBuffer.Length(), buffer, count);
	try
	{
		usbDevice.SendControlOutTransferAsync(setupPacket, writeBuffer).get();
	}
	catch (...)
	{
		printf("Error with writing to register");
		winrt::throw_last_error();	
	}
}

void I2cDriver::writeRegisterByte(uint16_t address, uint8_t reg, uint8_t value)
{
	writeRegister(address, reg, 1, &value);
}

void I2cDriver::writeRegisterByteMasked(uint16_t address, uint8_t reg, uint8_t value, uint8_t mask)
{
	if (mask == 0xff)
	{
		writeRegisterByte(address, reg, value);
	}
	else
	{
		uint8_t original = readRegisterByte(address, reg);
		writeRegisterByte(address, reg, (original & ~mask) | (value & mask));
	}
}

uint16_t I2cDriver::readRegister(uint16_t address, uint8_t reg, uint16_t count, uint8_t* buffer)
{
	UsbSetupPacket setupPacket;
	UsbControlRequestType requestType;
	requestType.AsByte(0xc0); // 1_10_00000
	setupPacket.RequestType(requestType);
	setupPacket.Request(VR_I2C_DATA_TRANSFER);
	setupPacket.Value(address);
	setupPacket.Index(reg);
	setupPacket.Length(count);

	// HACK just make this method use IBuffer eventually
	Buffer inReadBuffer(count);
	inReadBuffer.Length(count);
	memcpy_s(inReadBuffer.data(), inReadBuffer.Length(), buffer, count);

	try
	{
		auto readBuffer = usbDevice.SendControlInTransferAsync(setupPacket, inReadBuffer).get();

		memcpy_s(buffer, count, readBuffer.data(), readBuffer.Length());

		return (uint16_t)readBuffer.Length();
	}
	catch (...)
	{
		printf("Error %d\n", GetLastError());
		return 0;
	}
}

char I2cDriver::readRegisterByte(uint16_t address, uint8_t reg)
{
	uint8_t value;
	if (readRegister(address, reg, 1, &value) != 1)
	{
		throw;
	}

	return value;
}