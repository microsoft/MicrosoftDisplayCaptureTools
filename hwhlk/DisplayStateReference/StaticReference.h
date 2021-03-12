#pragma once
#include "StaticReference.g.h"

namespace winrt::DisplayStateReference::implementation
{
    struct StaticReference : StaticReferenceT<StaticReference>
    {
        StaticReference() = default;

        Windows::Data::Json::JsonObject GetSerializedData();
    };
}
