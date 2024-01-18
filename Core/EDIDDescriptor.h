#pragma once

namespace winrt::MicrosoftDisplayCaptureTools::Framework::Utilities 
{
// The configuration parameter used to specify what EDID to use in device bringup
const std::wstring c_EDIDOverrideParameter = L"TestingEDID";

// Create a simple EDID descriptor implementation that the framework itself will use when trying to determine display mappings.
struct EDIDDescriptor : winrt::implements<EDIDDescriptor, winrt::MicrosoftDisplayCaptureTools::Framework::IMonitorDescriptor>
{
    // Create the EDID with predefined data
    EDIDDescriptor(std::vector<uint8_t> data);
    EDIDDescriptor(winrt::Windows::Foundation::Collections::IVectorView<uint8_t> data);
    EDIDDescriptor(winrt::com_array<uint8_t> data);

    // IMonitorDescriptor APIs
    winrt::MicrosoftDisplayCaptureTools::Framework::MonitorDescriptorType Type()
    {
        return winrt::MicrosoftDisplayCaptureTools::Framework::MonitorDescriptorType::EDID;
    };
    winrt::Windows::Foundation::Collections::IVectorView<uint8_t> Data();

    // The only part of the EDID we currently allow modifying on the fly is the serial number, which is useful identifying display mappings
    uint32_t SerialNumber();
    void SerialNumber(uint32_t number);
    bool IsSame(winrt::MicrosoftDisplayCaptureTools::Framework::IMonitorDescriptor other);
    winrt::Windows::Foundation::Collections::IVector<uint8_t> GetRawData();

    static winrt::MicrosoftDisplayCaptureTools::Framework::IMonitorDescriptor CreateStandardEDID();
    static winrt::MicrosoftDisplayCaptureTools::Framework::IMonitorDescriptor CreateEDIDFromFile(hstring filePath);

private:
    void UpdateChecksum();

    winrt::Windows::Foundation::Collections::IVector<uint8_t> m_data;

    static const uint32_t MinEDIDSize = 128;
    static const uint32_t SerialNumberLocation = 12;
};

} // namespace winrt::MicrosoftDisplayCaptureTools::Framework::Utilities