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
        ConfigurationTools::ConfigurationToolRequirements Requirements();
        com_array<hstring> GetSupportedConfigurations();
        void SetConfiguration(hstring const& configuration);
        void ApplyToHardware(Windows::Devices::Display::Core::DisplayTarget const& target);
        void ApplyToReference(DisplayStateReference::IStaticReference const& reference);

    private:
        WireFormatToolConfigurations m_currentConfig;
    };
}