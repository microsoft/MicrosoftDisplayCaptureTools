#pragma once

#include "softgpuidl.h"
#include "softgpupeekidl.h"

static LPCWSTR softGPU_Api = L"SoftGpuApi2.dll";
static LPCWSTR softGPU_Peek = L"ZPeek.dll";

static LPCSTR softGPU_Api_create = "CreateSoftGpuApi";
static LPCSTR softGPU_Peek_create = "CreatePeekApi";

#pragma once

#include <mutex>
#include <memory>

//! \class Singleton
//! Base class implementation for creating a Singleton object that uses shared_ptr and weak_ptr.
template<typename TSingleton>
class Singleton abstract
{
public:
    static std::shared_ptr<TSingleton> Instance()
    {
        std::unique_lock<std::mutex> lock(s_lock);
        auto instance = s_instance.lock();
        if (nullptr == instance)
        {
            instance = std::make_shared<TSingleton>();
            s_instance = instance;
        }
        return instance;
    }

    Singleton(const Singleton&) = delete;
    const Singleton& operator=(const Singleton&) = delete;

protected:
    Singleton() = default;
    virtual ~Singleton() {};

private:
    static std::mutex s_lock;
    static std::weak_ptr<TSingleton> s_instance;
};

template<typename TSingleton>
std::mutex Singleton<TSingleton>::s_lock;

template<typename TSingleton>
std::weak_ptr<TSingleton> Singleton<TSingleton>::s_instance;
#pragma once

class SoftGpu : Singleton<SoftGpu>
{
public:
    static std::shared_ptr<SoftGpu> GetOrCreate()
    {
        return SoftGpu::Instance();
    }

    SoftGpu();
    ~SoftGpu();

    void GetSoftGpuApi(ISoftGpuApi** pApi);
    void GetSoftGpuConfig(ISoftGpuConfig** pApi);
    void GetSoftGpuAdapter(ISoftGpuAdapter** pAdapter);

    void AddMonitor(uint32_t id, ISoftGpuMonitor* pMonitor);
    void GetMonitor(uint32_t id, ISoftGpuMonitor** pMonitor);
    void RemoveMonitor(uint32_t id);

    void GetSoftGpuPeekApi(IPeekApi** pApi);

private:
    class SoftGpuLoader
    {
    public:
        SoftGpuLoader(LPCWSTR softGPUDir = L"C:\\SoftGPU");
        ~SoftGpuLoader();

        void CreateAPI(ISoftGpuApi** pApi);
        void CreatePeek(IPeekApi** pApi);

    private:
        HMODULE m_moduleApi, m_modulePeek;
    };

private:
    std::unique_ptr<SoftGpuLoader> m_loader;

    winrt::com_ptr<ISoftGpuApi> m_softGpuApi;
    winrt::com_ptr<ISoftGpuConfig> m_softGpuConfig;
    winrt::com_ptr<ISoftGpuAdapter> m_softGpuAdapter;

    winrt::com_ptr<IPeekApi> m_softGpuPeekApi;

    std::map<uint32_t, winrt::com_ptr<ISoftGpuMonitor>> m_monitorMap;
};