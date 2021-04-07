#pragma once

namespace winrt::ConfigurationTools::implementation
{
    enum class ResolutionToolConfigurations
    {
        _1080p,
        _2160p
    };

    struct ResolutionTool : implements<ResolutionTool, IConfigurationTool>
    {
        ResolutionTool();

        hstring Name();
        ConfigurationTools::ConfigurationToolCategory Category();
        ConfigurationTools::ConfigurationToolRequirements Requirements();
        com_array<hstring> GetSupportedConfigurations();
        void SetConfiguration(hstring const& configuration);
        void ApplyToHardware(Windows::Devices::Display::Core::DisplayTarget const& target);
        void ApplyToReference(DisplayStateReference::IStaticReference const& reference);

    private:
        ResolutionToolConfigurations m_currentConfig;
    };
}