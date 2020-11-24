#include "pch.h"

namespace winrt::HardwareCaptureTesting::Operations
{
    Tool* HardwareCaptureTesting::Operations::Tool::Create(uint32_t id)
    {
        switch (id)
        {
            //case 0:
            //    // instantiate tool
            //    return new SetMode();
            //    break;
        default:
            return nullptr;
        }
    }
}
