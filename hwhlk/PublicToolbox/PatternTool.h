#pragma once

namespace winrt::ConfigurationTools::implementation
{
    enum class PatternToolConfigurations
    {
        Black,
        White,
        Red,
        Green,
        Blue
    };

    struct PatternTool : implements<PatternTool, IConfigurationTool>
    {
        PatternTool();

        hstring Name();
        ConfigurationTools::ConfigurationToolCategory Category();
        ConfigurationTools::ConfigurationToolRequirements Requirements();
        com_array<hstring> GetSupportedConfigurations();
        void SetConfiguration(hstring const& configuration);
        void ApplyToHardware(Windows::Devices::Display::Core::DisplayTarget const& target);
        void ApplyToReference(DisplayStateReference::IStaticReference const& reference);

    private:
        void ApplyToSoftwareReferenceFallback(DisplayStateReference::IStaticReference const& reference);
        PatternToolConfigurations m_currentConfig;
    };
}