#include "pch.h"
#include "StaticReferenceData.h"

using namespace WEX::Common;
using namespace WEX::Logging;
using namespace WEX::TestExecution;

namespace winrt::DisplayStateReference::implementation
{
    StaticReferenceData::StaticReferenceData(hstring name) : m_name(name), m_buffersCreated(false)
    {
        m_frameInfo = {  };
    }
    hstring StaticReferenceData::Name()
    {
        return m_name;
    }
    Windows::Data::Json::JsonObject StaticReferenceData::GetSerializedData()
    {
        throw winrt::hresult_not_implemented();
    }
    Windows::Storage::Streams::IBuffer StaticReferenceData::GetNamedMetadata(hstring name)
    {
        auto entry = m_metadataMap.find(name);

        if (entry == m_metadataMap.end())
        {
            // there is no metadata stream by that name tracked here.
            return nullptr;
        }
        
        return entry->second;
    }
    void StaticReferenceData::AddNamedMetadata(hstring name, Windows::Storage::Streams::IBuffer buffer)
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
        if (m_buffersCreated)
        {
            //
            // if the buffers have already been created, the only frame changes allowed are for position. Size and format
            // should remain the same - tools can't change this after the frame has been created.
            //
            if (frameInfo.height != m_frameInfo.height ||
                frameInfo.width != m_frameInfo.width ||
                frameInfo.pixelStride != m_frameInfo.pixelStride ||
                frameInfo.pixelFormat != m_frameInfo.pixelFormat)
            {
                Log::Error(L"Buffers have already been created, no tool should be attempting to change state. Is this tool incorrectly categorized?");
            }
        }
        m_frameInfo = frameInfo;
    }
    Windows::Foundation::IMemoryBufferReference StaticReferenceData::GetFrameFromCPU()
    {
        if (!m_buffersCreated)
        {
            // The frame hasn't been used yet, so lazily construct it here.
            m_frameBuffer = Windows::Foundation::MemoryBuffer(m_frameInfo.height * m_frameInfo.width * m_frameInfo.pixelStride);
        }

        return m_frameBuffer.CreateReference();
    }
}
