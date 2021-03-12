#include "pch.h"
#include "StaticReference.h"
#include "StaticReference.g.cpp"

namespace winrt::DisplayStateReference::implementation
{
    Windows::Data::Json::JsonObject StaticReference::GetSerializedData()
    {
        throw hresult_not_implemented();
    }
}
