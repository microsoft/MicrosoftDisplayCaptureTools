#pragma once

namespace winrt::DisplayStateReference::implementation
{
    struct StaticReferenceData : implements<StaticReferenceData, IStaticReference>
    {
        StaticReferenceData(hstring name);

        hstring Name();

        Windows::Data::Json::JsonObject GetSerializedMetadata();

        Windows::Foundation::IMemoryBufferReference GetNamedMetadata(hstring name);

        void AddNamedMetadata(hstring name, Windows::Foundation::IMemoryBuffer buffer);

        FrameBasicInfo FrameInfo();

        void FrameInfo(FrameBasicInfo frameInfo);

        Windows::Graphics::Imaging::SoftwareBitmap StaticReferenceData::GetFrame();

    private:
        void CreateBitmap();

    private:
        
        hstring m_name;
        FrameBasicInfo m_frameInfo;
        std::map<hstring, Windows::Foundation::IMemoryBuffer> m_metadataMap;

        std::shared_ptr<Windows::Graphics::Imaging::SoftwareBitmap> m_frame;
    };
}
