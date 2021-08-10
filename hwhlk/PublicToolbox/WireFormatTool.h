#pragma once

#include "winrt/MicrosoftDisplayCaptureTools.ConfigurationTools.h"

namespace winrt::Toolbox::implementation
{
    enum class WireFormatToolConfigurations
    {
        RGB8
    };

    struct WireFormatTool : implements<WireFormatTool, MicrosoftDisplayCaptureTools::ConfigurationTools::IConfigurationTool>
    {
        WireFormatTool();

        hstring Name();
        MicrosoftDisplayCaptureTools::ConfigurationTools::ConfigurationToolCategory Category();
        MicrosoftDisplayCaptureTools::ConfigurationTools::ConfigurationToolRequirements Requirements();
        com_array<hstring> GetSupportedConfigurations();
        void SetConfiguration(hstring const& configuration);
        void ApplyToHardware(Windows::Devices::Display::Core::DisplayTarget const& target);
        void ApplyToReference(MicrosoftDisplayCaptureTools::DisplayStateReference::IStaticReference const& reference);

    private:
        WireFormatToolConfigurations m_currentConfig;
    };
}