#include "pch.h"
#include <filesystem>

namespace winrt 
{
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Windows::Foundation::Collections;
    using namespace winrt::Windows::Devices::Enumeration;
    using namespace winrt::Windows::Devices::Usb;
    using namespace winrt::Windows::Graphics::Imaging;
    using namespace winrt::Windows::Graphics::DirectX;
    using namespace winrt::Windows::Devices::Display;
    using namespace winrt::Windows::Devices::Display::Core;
    using namespace winrt::Windows::Storage;
    using namespace winrt::Windows::Storage::Streams;
    using namespace winrt::MicrosoftDisplayCaptureTools::Framework;
    using namespace winrt::TanagerPlugin::DisplayHelpers;
}

namespace winrt::TanagerPlugin::implementation 
{

struct HDMICapabilities : implements<HDMICapabilities, winrt::MicrosoftDisplayCaptureTools::CaptureCard::ICaptureCapabilities>
{
    HDMICapabilities() = default;

    bool CanReturnRawFramesToHost()
    {
        return true;
    }
    bool CanReturnFramesToHost()
    {
        return true;
    }
    bool CanCaptureFrameSeries()
    {
        return false;
    }
    bool CanHotPlug()
    {
        return true;
    }
    bool CanConfigureEDID()
    {
        return true;
    }
    bool CanConfigureDisplayID()
    {
        return false;
    }
    uint32_t GetMaxDescriptorSize()
    {
        return MaxDescriptorByteSize;
    }
};

TanagerDisplayInputHdmi::TanagerDisplayInputHdmi(std::weak_ptr<TanagerDevice> parent, winrt::ILogger const& logger) :
    m_parent(parent), m_logger(logger)
{
}

TanagerDisplayInputHdmi::~TanagerDisplayInputHdmi()
{
    m_logger.LogNote(winrt::hstring(L"Cleaning up TanagerDisplayInputHdmi: ") + Name());

    // HPD out
    if (auto parent = m_parent.lock())
    {
        auto lock = std::scoped_lock(parent->SelectHdmi());
        parent->FpgaWrite(0x4, std::vector<byte>({0x32})); // HPD high
    }
}

hstring TanagerDisplayInputHdmi::Name()
{
    return L"HDMI";
}

void TanagerDisplayInputHdmi::SetDescriptor(MicrosoftDisplayCaptureTools::Framework::IMonitorDescriptor descriptor)
{
    // Indicate that we have a new call to set the descriptor, after we HPD next - don't try to HPD again until
    // after the descriptor changes again.
    m_hasDescriptorChanged = true;

    if (descriptor.Type() != MicrosoftDisplayCaptureTools::Framework::MonitorDescriptorType::EDID)
    {
        m_logger.LogAssert(L"Only EDID descriptors are currently supported.");
    }

    auto edidDataView = descriptor.Data();
    std::vector<byte> edid(edidDataView.begin(), edidDataView.end());

    SetEdid(edid);
}

MicrosoftDisplayCaptureTools::CaptureCard::ICaptureTrigger TanagerDisplayInputHdmi::GetCaptureTrigger()
{
    return MicrosoftDisplayCaptureTools::CaptureCard::ICaptureTrigger();
}

MicrosoftDisplayCaptureTools::CaptureCard::ICaptureCapabilities TanagerDisplayInputHdmi::GetCapabilities()
{
    return winrt::make<HDMICapabilities>();
}

MicrosoftDisplayCaptureTools::CaptureCard::IDisplayCapture TanagerDisplayInputHdmi::CaptureFrame()
{
    auto parent = m_parent.lock();
    if (!parent)
    {
        m_logger.LogError(L"Cannot obtain reference to Tanager device.");
    }

    auto lock = std::scoped_lock(parent->SelectHdmi());

    // Reset the DRAM controller in the FPGA
    parent->FpgaWrite(0x30, std::vector<byte>({1}));
    std::vector<byte> video_register_vector = parent->FpgaRead(0x30, 1);

    if (video_register_vector.empty())
    {
        m_logger.LogError(L"Failed to read DRAM controller registers.");
    }

    if ((video_register_vector[0] & 0x01) == 0)
    {
        m_logger.LogError(L"DRAM controller register reset bit not set.");
    }

    // Wait for reset to complete
    uint32_t loopCount = 0;
    constexpr uint32_t loopLimit = 50;
    video_register_vector = parent->FpgaRead(0x30, 1);
    while ((video_register_vector[0] & 0x03) != 0x03 && loopCount++ < loopLimit)
    {
        video_register_vector = parent->FpgaRead(0x30, 1);
        Sleep(20);
    }
    if ((video_register_vector[0] & 0x03) != 0x03)
    {
        m_logger.LogError(L"DRAM controller did not reset in time allowed.");
    }

    // Clear the FPGA DRAM controller register
    parent->FpgaWrite(0x30, std::vector<byte>({0}));
    video_register_vector = parent->FpgaRead(0x30, 1);
    if (video_register_vector[0] != 0)
    {
        m_logger.LogError(L"DRAM controller register did not zero.");
    }

    // Check to see if ITE chip is locked
    auto locked = parent->IsVideoLocked();
    if (!locked)
    {
        m_logger.LogError(L"Video is not locked");
    }

    // Capture frame in DRAM
    parent->FpgaWrite(0x20, std::vector<byte>({0}));

    // Give the Tanager time to capture a frame.
    loopCount = 0;
    video_register_vector = parent->FpgaRead(0x20, 1);
    while (video_register_vector.size() > 0 && (video_register_vector[0] & 0x01) == 0x00 && loopCount++ < loopLimit)
    {
        video_register_vector = parent->FpgaRead(0x20, 1);
        Sleep(20);
    }

    if (video_register_vector.size() == 0)
    {
        m_logger.LogError(L"Zero bytes of data returned from FpgaRead.");
    }
    if (loopCount >= loopLimit)
    {
        m_logger.LogError(L"Timeout while waiting for video frame capture to complete.");
    }

    // query resolution
    auto timing = parent->GetVideoTiming();

    // compute size of buffer
    uint32_t bufferSizeInDWords = timing.hActive * timing.vActive; // For now, assume good sync and 4 bytes per pixel

    // FX3 requires the read size to be a multiple of 2048 DWORDs
    if (bufferSizeInDWords % 2048)
    {
        bufferSizeInDWords += 2048 - (bufferSizeInDWords % 2048);
    }

    // specify number of dwords to read
    // This is bufferSizeInDWords but in big-endian order in a vector<byte>
    parent->FpgaWrite(
        0x15,
        std::vector<byte>(
            {(uint8_t)((bufferSizeInDWords >> 24) & 0xff),
             (uint8_t)((bufferSizeInDWords >> 16) & 0xff),
             (uint8_t)((bufferSizeInDWords >> 8) & 0xff),
             (uint8_t)(bufferSizeInDWords & 0xff)}));

    // prepare for reading
    parent->FpgaWrite(0x10, std::vector<byte>({3}));

    // initiate read sequencer
    parent->FpgaWrite(0x10, std::vector<byte>({2}));

    // read frame
    auto frameData = parent->ReadEndPointData(bufferSizeInDWords * 4);

    // turn off read sequencer
    parent->FpgaWrite(0x10, std::vector<byte>({3}));

    // Add any extended properties that aren't directly exposable in the IDisplayCapture* interfaces yet
    auto extendedProps = winrt::multi_threaded_observable_map<winrt::hstring, winrt::IInspectable>();
    extendedProps.Insert(L"pixelClock", winrt::box_value(timing.pixelClock));
    extendedProps.Insert(L"hTotal", winrt::box_value(timing.hTotal));
    extendedProps.Insert(L"hFrontPorch", winrt::box_value(timing.hFrontPorch));
    extendedProps.Insert(L"hSyncWidth", winrt::box_value(timing.hSyncWidth));
    extendedProps.Insert(L"hBackPorch", winrt::box_value(timing.hBackPorch));
    extendedProps.Insert(L"vTotal", winrt::box_value(timing.vTotal));
    extendedProps.Insert(L"vFrontPorch", winrt::box_value(timing.vFrontPorch));
    extendedProps.Insert(L"vSyncWidth", winrt::box_value(timing.vSyncWidth));
    extendedProps.Insert(L"vBackPorch", winrt::box_value(timing.vBackPorch));

    auto aviInfoFrame = parent->GetAviInfoframe();
    winrt::Buffer infoFrame(ARRAYSIZE(aviInfoFrame.data));
    memcpy(infoFrame.data(), aviInfoFrame.data, infoFrame.Capacity());
    extendedProps.Insert(L"InfoFrame", infoFrame);

    auto resolution = winrt::Windows::Graphics::SizeInt32();
    resolution = {timing.hActive, timing.vActive};

    return winrt::make<TanagerDisplayCapture>(frameData, resolution, extendedProps, m_logger);
}

void TanagerDisplayInputHdmi::FinalizeDisplayState()
{
    if (auto parent = m_parent.lock())
    {
        if (m_hasDescriptorChanged || !m_strongParent)
        {
            auto lock = std::scoped_lock(parent->SelectHdmi());

            auto hasDeviceChanged = WaitForDisplayDevicesChange();
            parent->FpgaWrite(0x4, std::vector<byte>({0x30})); // HPD high

            // We have HPD'd in a display, since we need to be able to HPD out again - take a strong reference on the 'parent'
            // to ensure it isn't cleaned up before this input HPD's out.
            m_strongParent = parent;

            // Reset the descriptor guard bool here to make sure that we won't HPD in again unless the descriptor changes
            m_hasDescriptorChanged = false;

            if (AsyncStatus::Completed != hasDeviceChanged.wait_for(std::chrono::seconds(8)))
            {
                m_logger.LogError(L"Did not detect a new device being plugged in after hotplugging HDMI");
            }
        }
    }
    else
    {
        m_logger.LogAssert(L"Cannot obtain reference to Tanager object.");
    }
}

void TanagerDisplayInputHdmi::SetEdid(std::vector<byte> edid)
{
    // EDIDs are made of a series of 128-byte blocks
    if (edid.empty() || edid.size() % 128 != 0)
    {
        m_logger.LogError(L"SetEdid provided edid of invalid size=" + to_hstring(edid.size()));
    }

    // Ensure under max descriptor size
    if (edid.size() > MaxDescriptorByteSize)
    {
        m_logger.LogError(L"SetEdid provided a too large edid, size=" + to_hstring(edid.size()));
    }

    unsigned short writeAddress = 0x400;

    if (auto parent = m_parent.lock())
    {
        auto lock = std::scoped_lock(parent->SelectHdmi());
        parent->FpgaWrite(writeAddress, edid);
    }
    else
    {
        m_logger.LogAssert(L"Cannot obtain reference to Tanager object.");
    }
}
} // namespace winrt::TanagerPlugin::implementation