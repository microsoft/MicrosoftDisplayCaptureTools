#include "pch.h"

namespace winrt::HardwareCaptureTesting::Operations
{
    Tool* HardwareCaptureTesting::Operations::Tool::Create(winrt::hstring name)
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
