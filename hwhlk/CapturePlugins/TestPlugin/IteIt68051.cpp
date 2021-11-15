#include "pch.h"
#include "IteIt68051.h"

IteIt68051::IteIt68051(unsigned char address, std::shared_ptr<I2cDriver> pDriver)
	: address(address), 
	  pDriver(pDriver)
{
}

IteIt68051::~IteIt68051()
{
}
