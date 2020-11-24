#include "pch.h"
#include "Toolbox.h"
#include "Toolbox.g.cpp"

namespace winrt::HardwareCaptureTesting::Operations::implementation
{
    Toolbox::Toolbox(Windows::Devices::Display::Core::DisplayAdapter const& adapter, Windows::Devices::Display::Core::DisplayPath const& displayPath)
    {
        throw hresult_not_implemented();
    }
    Windows::Foundation::Collections::IMapView<hstring, HardwareCaptureTesting::Operations::ToolCategory> Toolbox::GetSupportedTools()
    {
        throw hresult_not_implemented();
    }
    void Toolbox::CreateTool(hstring const& toolName, hstring const& toolParams)
    {
        throw hresult_not_implemented();
    }
    void Toolbox::ExerciseTool(hstring const& toolName)
    {
        throw hresult_not_implemented();
    }
}
