#pragma once
namespace winrt::TanagerPlugin::DisplayHelpers
{
    // Wait until windows registers that a display device change has happened.
	winrt::Windows::Foundation::IAsyncAction WaitForDisplayDevicesChange();
}