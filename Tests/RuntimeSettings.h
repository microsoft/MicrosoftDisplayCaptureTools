namespace RuntimeSettings {
    /// <summary>
    /// The set of flags which can be set to configure how this component will behave.
    /// </summary>
    struct RuntimeSettings : winrt::implements<RuntimeSettings, winrt::MicrosoftDisplayCaptureTools::Framework::IRuntimeSettings>
    {
        RuntimeSettings() = default;

        winrt::Windows::Foundation::IInspectable GetSettingValue(winrt::hstring name);
        bool GetSettingValueAsBool(winrt::hstring name);
        winrt::hstring GetSettingValueAsString(winrt::hstring name);
        double GetSettingValueAsDouble(winrt::hstring name);
    };

    winrt::MicrosoftDisplayCaptureTools::Framework::IRuntimeSettings GetRuntimeSettings();
} // namespace RuntimeSettings