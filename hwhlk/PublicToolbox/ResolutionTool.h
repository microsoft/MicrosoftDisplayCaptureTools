#pragma once

#include "winrt/MicrosoftDisplayCaptureTools.ConfigurationTools.h"

namespace winrt::Toolbox::implementation
{
    enum class ResolutionToolConfigurations
    {
        _1080p,
        _2160p
    };

    struct ResolutionTool : implements<ResolutionTool, MicrosoftDisplayCaptureTools::ConfigurationTools::IConfigurationTool>
    {
        ResolutionTool();

        hstring Name();
        MicrosoftDisplayCaptureTools::ConfigurationTools::ConfigurationToolCategory Category();
        MicrosoftDisplayCaptureTools::ConfigurationTools::ConfigurationToolRequirements Requirements();
        com_array<hstring> GetSupportedConfigurations();
        void SetConfiguration(hstring const& configuration);
        void ApplyToHardware(Windows::Devices::Display::Core::DisplayTarget const& target);
        void ApplyToReference(MicrosoftDisplayCaptureTools::DisplayStateReference::IStaticReference const& reference);

    private:
        ResolutionToolConfigurations m_currentConfig;
    };
}