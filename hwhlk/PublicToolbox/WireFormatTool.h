#pragma once

namespace winrt::ConfigurationTools::implementation
{
    enum class WireFormatToolConfigurations
    {
        RGB8
    };

    struct WireFormatTool : implements<WireFormatTool, IConfigurationTool>
    {
        WireFormatTool();

        hstring Name();
        ConfigurationTools::ConfigurationToolCategory Category();
        void Category(ConfigurationTools::ConfigurationToolCategory const& value);
        ConfigurationTools::ConfigurationToolRequirements Requirements();
        void Requirements(ConfigurationTools::ConfigurationToolRequirements const& value);
        com_array<hstring> GetSupportedConfigurations();
        void SetConfiguration(hstring const& configuration);
        void ApplyToHardware(Windows::Devices::Display::Core::DisplayTarget const& target);
        void ApplyToSoftwareReference(DisplayStateReference::IStaticReference const& reference);

    private:
        WireFormatToolConfigurations m_currentConfig;
    };
}