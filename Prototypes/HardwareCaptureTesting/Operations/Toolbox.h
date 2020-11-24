#pragma once
#include "pch.h"

namespace winrt::HardwareCaptureTesting::Operations::implementation
{
    struct Toolbox : ToolboxT<Toolbox>
    {
        Toolbox() = default;

        Toolbox(Windows::Devices::Display::Core::DisplayAdapter const& adapter, Windows::Devices::Display::Core::DisplayPath const& displayPath);
        Windows::Foundation::Collections::IMapView<hstring, HardwareCaptureTesting::Operations::ToolCategory> GetSupportedTools();
        void CreateTool(hstring const& toolName, hstring const& toolParams);
        void ExerciseTool(hstring const& toolName);
    };
}
namespace winrt::HardwareCaptureTesting::Operations::factory_implementation
{
    struct Toolbox : ToolboxT<Toolbox, implementation::Toolbox>
    {
    };
}
