#include "pch.h"
#include "StaticReferenceData.h"

#include <MemoryBuffer.h>

#include <windows.ui.composition.interop.h>

using namespace WEX::Common;
using namespace WEX::Logging;
using namespace WEX::TestExecution;

namespace winrt::DisplayStateReference::implementation
{
    StaticReferenceData::StaticReferenceData(hstring name) : m_name(name)
    {
        m_frameInfo = {  };
    }
    hstring StaticReferenceData::Name()
    {
        return m_name;
    }
    Windows::Data::Json::JsonObject StaticReferenceData::GetSerializedMetadata()
    {
        // TODO: serialize any set metadata into a JSON object
        return Windows::Data::Json::JsonObject();
    }
    Windows::Foundation::IMemoryBufferReference StaticReferenceData::GetNamedMetadata(hstring name)
    {
        auto entry = m_metadataMap.find(name);

        if (entry == m_metadataMap.end())
        {
            // there is no metadata stream by that name tracked here.
            return nullptr;
        }

        return entry->second.CreateReference();
    }
    void StaticReferenceData::AddNamedMetadata(hstring name, Windows::Foundation::IMemoryBuffer buffer)
    {
        auto entry = m_metadataMap.find(name);

        if (entry == m_metadataMap.end())
        {
            m_metadataMap.insert({ name, buffer });
        }
        else
        {
            entry->second = buffer;
        }
    }
    FrameBasicInfo StaticReferenceData::FrameInfo()
    {
        return m_frameInfo;
    }
    void StaticReferenceData::FrameInfo(FrameBasicInfo frameInfo)
    {
        if (m_frame)
        {
            //
            // if the buffers have already been created, the only frame changes allowed are for position. Size and format
            // should remain the same - tools can't change this after the frame has been created.
            //
            if (frameInfo.height != m_frameInfo.height ||
                frameInfo.width != m_frameInfo.width ||
                frameInfo.format != m_frameInfo.format)
            {
                Log::Error(L"Buffers have already been created, no tool should be attempting to change state. Is this tool incorrectly categorized?");
            }
        }

        m_frameInfo = frameInfo;
    }
    Windows::Graphics::Imaging::SoftwareBitmap StaticReferenceData::GetFrame()
    {
        if (!m_frame)
        {
            CreateBitmap();
        }

        return *m_frame;
    }
    void StaticReferenceData::CreateBitmap()
    {
        m_frame = std::make_shared<winrt::Windows::Graphics::Imaging::SoftwareBitmap>(
            m_frameInfo.format,
            m_frameInfo.width,
            m_frameInfo.height,
            winrt::Windows::Graphics::Imaging::BitmapAlphaMode::Ignore
            );
    }
}
