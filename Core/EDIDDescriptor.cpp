#include "pch.h"
#include "EDIDDescriptor.h"

namespace winrt 
{
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Windows::Foundation::Collections;
    using namespace winrt::MicrosoftDisplayCaptureTools::Framework;
} // namespace winrt

namespace winrt::MicrosoftDisplayCaptureTools::Framework::Utilities {

EDIDDescriptor::EDIDDescriptor(std::vector<uint8_t> data)
{
    if (data.size() < MinEDIDSize)
    {
        throw winrt::hresult_invalid_argument();
    }

    m_data = winrt::single_threaded_vector<uint8_t>(std::move(data));
}

EDIDDescriptor::EDIDDescriptor(winrt::com_array<uint8_t> data)
{
    if (data.size() < MinEDIDSize)
    {
        throw winrt::hresult_invalid_argument();
    }

    std::vector<uint8_t> vec(data.begin(), data.end());

    m_data = winrt::single_threaded_vector<uint8_t>(std::move(vec));
}

winrt::IVectorView<uint8_t> EDIDDescriptor::Data()
{
    return m_data.GetView();
}

uint32_t EDIDDescriptor::SerialNumber()
{
    uint32_t serialNumber = (m_data.GetAt(SerialNumberLocation + 0) << 24) + (m_data.GetAt(SerialNumberLocation + 1) << 16) +
                            (m_data.GetAt(SerialNumberLocation + 2) << 8) + m_data.GetAt(SerialNumberLocation + 3);

    return serialNumber;
}

void EDIDDescriptor::SerialNumber(uint32_t number)
{
    m_data.SetAt(SerialNumberLocation + 3, static_cast<uint8_t>(number));
    m_data.SetAt(SerialNumberLocation + 2, static_cast<uint8_t>(number >> 8));
    m_data.SetAt(SerialNumberLocation + 1, static_cast<uint8_t>(number >> 16));
    m_data.SetAt(SerialNumberLocation + 0, static_cast<uint8_t>(number >> 24));

    UpdateChecksum();
}

void EDIDDescriptor::UpdateChecksum()
{
    uint8_t sum = 0;

    for (auto i = 0; i < MinEDIDSize - 1; i++)
    {
        sum += m_data.GetAt(i);
    }

    // EDID checksums are just the final byte of the block, and are set such that the sum of the block = 0 (mod 256)
    uint8_t checksum = (uint8_t)((0x100 - sum) & 0xFF);

    m_data.SetAt(MinEDIDSize - 1, checksum);
}

winrt::IMonitorDescriptor EDIDDescriptor::CreateStandardEDID()
{
    return winrt::make<EDIDDescriptor>(std::vector<uint8_t>(
        {0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x36, 0x68, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1E, 0x1E, 0x01,
         0x04, 0xA3, 0x3C, 0x22, 0x78, 0x3A, 0x48, 0x15, 0xA7, 0x56, 0x52, 0x9C, 0x27, 0x0F, 0x50, 0x54, 0xA5, 0x4B, 0x00,
         0x71, 0x4F, 0x81, 0x80, 0xA9, 0xC0, 0xD1, 0xC0, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x3A, 0x80,
         0x18, 0x71, 0x38, 0x2D, 0x40, 0x58, 0x2C, 0x45, 0x00, 0x56, 0x50, 0x21, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0xFF,
         0x00, 0x36, 0x56, 0x54, 0x48, 0x5A, 0x31, 0x33, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xFC, 0x00,
         0x44, 0x45, 0x4C, 0x4C, 0x20, 0x50, 0x32, 0x37, 0x31, 0x39, 0x48, 0x0A, 0x20, 0x00, 0x00, 0x00, 0xFD, 0x00, 0x38,
         0x4C, 0x1E, 0x53, 0x11, 0x01, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0xD7}));
}

} // namespace winrt::MicrosoftDisplayCaptureTools::Framework::Utilities