#pragma once

namespace winrt::DisplayStateReference::implementation
{
    struct StaticReferenceData : implements<StaticReferenceData, IStaticReference>
    {
        StaticReferenceData() = default;

        Windows::Data::Json::JsonObject GetSerializedData();
    };
}
