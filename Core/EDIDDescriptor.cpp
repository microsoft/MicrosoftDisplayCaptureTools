#include "pch.h"
#include "EDIDDescriptor.h"
#include "MicrosoftDisplayCaptureTools.h"

#include <fstream>

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

bool EDIDDescriptor::IsSame(winrt::IMonitorDescriptor other)
{
    if (Type() != other.Type())
    {
        return false;
    }

    auto otherData = other.Data();

    if (m_data.Size() != otherData.Size())
    {
        return false;
    }

    for (uint32_t i = 0; i < m_data.Size(); i++)
    {
        if (m_data.GetAt(i) != otherData.GetAt(i))
        {
            return false;
        }
    }

    return true;
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
    return CreateEDIDFromFile(L"StandardEDID.bin");
}

winrt::IMonitorDescriptor EDIDDescriptor::CreateEDIDFromFile(hstring filePath)
{
    // Attempt to load the file given either as a fully qualified path or from the cwd
    std::ifstream file(filePath.c_str());

    if (!file)
    {
        // Attempt to load the file as a relative path to this current dll
        wchar_t currentFilePath[MAX_PATH];
        auto coreHandle = GetModuleHandle(winrt::MicrosoftDisplayCaptureTools::Framework::implementation::c_CoreFrameworkName.c_str());
        auto returnLength = GetModuleFileNameW(coreHandle, currentFilePath, MAX_PATH);

        if (returnLength == MAX_PATH)
        {
            // The file could not be located.
            throw winrt::hresult_invalid_argument();
        }

        std::filesystem::path currentPath(currentFilePath);
        auto localDirectory = currentPath.parent_path();

        auto edidFilePath = localDirectory / filePath.c_str();

        file = std::ifstream(edidFilePath);

        if (!file)
        {
            // The file could not be located.
            throw winrt::hresult_invalid_argument();
        }
    }

    std::vector<uint8_t> fileData(std::istreambuf_iterator<char>(file), {});
    return winrt::make<EDIDDescriptor>(fileData);
}

} // namespace winrt::MicrosoftDisplayCaptureTools::Framework::Utilities