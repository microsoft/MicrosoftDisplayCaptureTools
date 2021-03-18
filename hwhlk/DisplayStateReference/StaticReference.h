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
namespace winrt::DisplayStateReference::factory_implementation
{
    struct StaticReference : StaticReferenceT<StaticReference, implementation::StaticReference>
    {
    };
}