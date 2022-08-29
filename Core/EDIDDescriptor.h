#pragma once

namespace winrt::MicrosoftDisplayCaptureTools::Framework::Utilities 
{

// Create a simple EDID descriptor implementation that the framework itself will use when trying to determine display mappings.
struct EDIDDescriptor : winrt::implements<EDIDDescriptor, winrt::MicrosoftDisplayCaptureTools::Framework::IMonitorDescriptor>
{
    // Create the EDID with predefined data
    EDIDDescriptor(std::vector<uint8_t> data);
    EDIDDescriptor(winrt::com_array<uint8_t> data);

    winrt::MicrosoftDisplayCaptureTools::Framework::MonitorDescriptorType Type()
    {
        return winrt::MicrosoftDisplayCaptureTools::Framework::MonitorDescriptorType::EDID;
    };

    // The only part of the EDID we currently allow modifying on the fly is the serial number, which is useful identifying display mappings
    uint32_t SerialNumber();
    void SerialNumber(uint32_t number);

    winrt::Windows::Foundation::Collections::IVectorView<uint8_t> Data();

    static winrt::MicrosoftDisplayCaptureTools::Framework::IMonitorDescriptor CreateStandardEDID();

private:
    void UpdateChecksum();

    winrt::Windows::Foundation::Collections::IVector<uint8_t> m_data;
    winrt::MicrosoftDisplayCaptureTools::Framework::ILogger m_logger;

    static const uint32_t MinEDIDSize = 128;
    static const uint32_t SerialNumberLocation = 12;
};

} // namespace winrt::MicrosoftDisplayCaptureTools::Framework::Utilities