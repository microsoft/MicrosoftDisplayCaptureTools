/*
* Copyright (c) Microsoft Corporation. All rights reserved.
*/
#pragma once
#include "TE.Common.h"

#include "ITestResource.h"

#pragma push_macro("_In_")
#pragma push_macro("_Outptr_")

#if !defined(_In_)
#define _In_
#endif

#if !defined(_Outptr_)
#define _Outptr_
#endif

namespace WEX { namespace TestExecution
{
    class ResourceListImpl;

    // Interface to add TestResources to the list in the BuildResourceList implementation
    class TECOMMON_API ResourceList final
    {
        friend struct ResourceListFactory;
        UNIT_TEST_CLASS(WEX::UnitTests::TestResourceDataSourceTests);
        UNIT_TEST_CLASS(WEX::UnitTests::DynamicTreeTests);
    public:
        HRESULT Add(_In_ ITestResource* pTestResource);
        ULONG Count();
        HRESULT Item(ULONG index, _Outptr_ ITestResource** ppTestResource);

    private:
        ResourceList();
        ~ResourceList();
        ResourceList(const ResourceList&) = delete;
        ResourceList& operator=(const ResourceList&) = delete;
    };

    // Class to retrieve the TestResources for the current invocation of the test method in concern.
    class TECOMMON_API Resources final
    {
    public:
        static ULONG __stdcall Count();
        static HRESULT __stdcall Item(size_t index, _Outptr_ ITestResource** ppTestResource);

    private:
        Resources() = delete;
        ~Resources() = delete;
        Resources(const Resources&) = delete;
        Resources& operator=(const Resources&) = delete;
    };

    // Internal definition for BuildResourceList
    typedef HRESULT (__cdecl *BuildResourceListFunction)(ResourceList&);
}/* namespace TestExecution */}/* namespace WEX */

// dll-exported function that users implement for each TestResource dependent test module - should be out of all namespaces
extern "C" __declspec(dllexport) HRESULT __cdecl BuildResourceList(WEX::TestExecution::ResourceList& resouceList);

#pragma pop_macro("_In_")
#pragma pop_macro("_Outptr_")
