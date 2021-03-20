#pragma once

namespace winrt::DisplayStateReference::implementation
{
    struct StaticReferenceData : implements<StaticReferenceData, IStaticReference>
    {
        StaticReferenceData(hstring name);

        hstring Name();

        Windows::Data::Json::JsonObject GetSerializedData();

        Windows::Storage::Streams::IBuffer GetNamedMetadata(hstring name);

        void AddNamedMetadata(hstring name, Windows::Storage::Streams::IBuffer buffer);

        FrameBasicInfo FrameInfo();

        void FrameInfo(FrameBasicInfo frameInfo);

        Windows::Foundation::IMemoryBufferReference GetFrameFromCPU();

    private:
        hstring m_name;
        FrameBasicInfo m_frameInfo;
        boolean m_buffersCreated;
        Windows::Foundation::IMemoryBuffer m_frameBuffer;
        std::map<hstring, Windows::Storage::Streams::IBuffer> m_metadataMap;
    };
}
