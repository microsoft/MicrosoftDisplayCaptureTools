#pragma once
#include <unknwn.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include "winrt/MicrosoftDisplayCaptureTools.Display.h"

namespace winrt::MicrosoftDisplayCaptureTools::Libraries
{
    // IWrapper - provides a mechanism to smuggle COM objects back and forth across the ABI between the different 
    // projects in this solution.
    struct __declspec(uuid("EAF394E2-BFED-45F6-BB4E-5E2785228C21")) IWrapper : ::IUnknown
    {
        virtual HRESULT __stdcall GetWrappedPointer(IUnknown** wrapped) = 0;
    };

    // Wrapper - provides an intersection of winrt objects and traditional, pre-existing COM objects. This allows C++
    // based tools to consume/add/modify COM objects used by the DisplayEngine rendering pipeline. When constructed
    // with a reference to a COM object, this will take ownership of that object and increase the refcount. The created
    // object will remain alive until specifically removed from the displayEngine property bag.
    struct Wrapper : winrt::implements<Wrapper, winrt::Windows::Foundation::IInspectable, IWrapper>
    {
        Wrapper(IUnknown* ptr)
        {
            m_ptr.attach(ptr);
            m_ptr->AddRef();
        }

        HRESULT __stdcall GetWrappedPointer(IUnknown** factory) noexcept override
        {
            if (!factory)
                return E_INVALIDARG;
            if (!m_ptr)
                return E_ABORT;

            *factory = m_ptr.get();
            return S_OK;
        }

    private:
        winrt::com_ptr<IUnknown> m_ptr;
    };

    // Take an item from the property bag and return it as the templated type. This is specifically intended to be used
    // with items stored in the property bag using the above com wrapper.
    template <typename T>
    static T* GetMapEntry(winrt::MicrosoftDisplayCaptureTools::Display::IDisplayEnginePlaneProperties propertyMap, winrt::hstring propertyName)
    {
        ::IUnknown* ptr = nullptr;
        winrt::check_hresult(propertyMap.Properties().Lookup(propertyName).as<IWrapper>()->GetWrappedPointer(&ptr));

        T* output = nullptr;
        winrt::check_hresult(ptr->QueryInterface<T>(&output));

        return output;
    }
} // namespace winrt::MicrosoftDisplayCaptureTools::Libraries