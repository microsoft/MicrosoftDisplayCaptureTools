#include "pch.h"
#include "SoftGpuLoader.h"

SoftGpu::SoftGpuLoader::SoftGpuLoader(LPCWSTR softGPUDir) :
    m_moduleApi(NULL),
    m_modulePeek(NULL)
{
    if (!SetCurrentDirectory(softGPUDir))
        winrt::throw_last_error();

    if (!(m_moduleApi = LoadLibrary(softGPU_Api)))
        winrt::throw_last_error();

    if (!(m_modulePeek = LoadLibrary(softGPU_Peek)))
        winrt::throw_last_error();
}

SoftGpu::SoftGpuLoader::~SoftGpuLoader()
{
    if (m_moduleApi) FreeLibrary(m_moduleApi);
    if (m_modulePeek) FreeLibrary(m_modulePeek);
}

void SoftGpu::SoftGpuLoader::CreateAPI(ISoftGpuApi** pApi)
{
    typedef HRESULT(WINAPI* CreateApiFunc)(ISoftGpuApi** pApi);

    CreateApiFunc createApi = (CreateApiFunc)GetProcAddress(m_moduleApi, softGPU_Api_create);

    if (!createApi)
        winrt::throw_last_error();

    HRESULT hr = createApi(pApi);

    if (FAILED(hr))
        winrt::throw_hresult(hr);
}

void SoftGpu::SoftGpuLoader::CreatePeek(IPeekApi** pApi)
{
    typedef HRESULT(WINAPI* CreatePeekFunc)(IPeekApi** pApi);

    CreatePeekFunc createApi = (CreatePeekFunc)GetProcAddress(m_modulePeek, softGPU_Peek_create);

    if (!createApi)
        winrt::throw_last_error();

    HRESULT hr = createApi(pApi);

    if (FAILED(hr))
        winrt::throw_hresult(hr);
}


struct SoftGpuCreateParams : SOFTGPU_CREATE_PARAMS
{
    SoftGpuCreateParams() = default;
    SoftGpuCreateParams(const SoftGpuCreateParams&) = default;
    SoftGpuCreateParams& operator=(SoftGpuCreateParams const&) = default;
    SoftGpuCreateParams(SoftGpuCreateParams&&) = default;
    SoftGpuCreateParams& operator=(SoftGpuCreateParams&&) = default;

    SoftGpuCreateParams(WDDM_VERSION_ wddmVersion,
        UINT monitorCount,
        HYBRID_MODE_ hybridMode,
        BOOL DefaultMonitor)
        : SOFTGPU_CREATE_PARAMS()
    {
        SoftGpuDriverType = monitorCount > 0 ? SOFTGPU_DEVID_FULL_D3D12_ : SOFTGPU_DEVID_RENDER_ONLY_;
        WddmVersion = wddmVersion;
        HybridMode = hybridMode;
        bBootPersisted = FALSE;
        NumDisplaySources = monitorCount;
        NumDisplayTargets = monitorCount;
        VmsSupported = TRUE;
        CreateDefaultMonitor = DefaultMonitor;
        BasicMpoSupport = FALSE;
        VidMemSize = 256 * 1024 * 1024;
        ApertureSize = 0x10000000; // An aperture of > 200MB is required for DFlip support.
        GpuMode = GPU_MODE_GPUMMU_;
    }

    SoftGpuCreateParams(WDDM_VERSION_ wddmVersion,
        UINT monitorCount,
        HYBRID_MODE_ hybridMode)
        : SoftGpuCreateParams(wddmVersion, monitorCount, hybridMode, monitorCount > 0)
    {
    }
};

void CreateSoftGpuAdapter(_In_ ISoftGpuConfig* pSoftGpuSystemConfig, SoftGpuCreateParams params, _Out_ ISoftGpuAdapter** ppSoftGpuAdapter)
{
    HRESULT hr = pSoftGpuSystemConfig->CreateSoftGpuAdapter(L"Test Adapter", dynamic_cast<SOFTGPU_CREATE_PARAMS*>(&params), ppSoftGpuAdapter);

    if (FAILED(hr))
        winrt::throw_hresult(hr);

    hr = pSoftGpuSystemConfig->SetActive(TRUE);

    if (FAILED(hr))
        winrt::throw_hresult(hr);
}

SoftGpu::SoftGpu() :
    m_softGpuApi(nullptr),
    m_softGpuConfig(nullptr),
    m_softGpuAdapter(nullptr)
{
    m_loader = std::make_unique<SoftGpuLoader>();

    HRESULT hr = S_OK;
    m_loader->CreateAPI(m_softGpuApi.put());

    if (FAILED(hr = m_softGpuApi->CreateSystemConfig(m_softGpuConfig.put())))
        winrt::throw_hresult(hr);

    SoftGpuCreateParams softGpuParams = SoftGpuCreateParams(WDDM_VLATEST_, 1, HYBRID_NONE_, FALSE);

    CreateSoftGpuAdapter(m_softGpuConfig.get(), softGpuParams, m_softGpuAdapter.put());

    m_loader->CreatePeek(m_softGpuPeekApi.put());
}

SoftGpu::~SoftGpu()
{
    if (m_softGpuApi) m_softGpuApi.detach();
    if (m_softGpuAdapter) m_softGpuAdapter.detach();
    if (m_softGpuConfig) m_softGpuConfig.detach();
}

void SoftGpu::GetSoftGpuPeekApi(IPeekApi** pApi)
{
    *pApi = m_softGpuPeekApi.get();
}

void SoftGpu::GetSoftGpuApi(ISoftGpuApi** pApi)
{
    *pApi = m_softGpuApi.get();
}

void SoftGpu::GetSoftGpuConfig(ISoftGpuConfig** pApi)
{
    *pApi = m_softGpuConfig.get();
}

void SoftGpu::GetSoftGpuAdapter(ISoftGpuAdapter** pAdapter)
{
    *pAdapter = m_softGpuAdapter.get();
}

void SoftGpu::AddMonitor(uint32_t id, ISoftGpuMonitor* pMonitor)
{
    if (m_monitorMap.find(id) != m_monitorMap.end())
    {
        winrt::com_ptr<ISoftGpuMonitor> monitor;
        monitor.attach(pMonitor);

        m_monitorMap.insert(
            std::pair<uint32_t, winrt::com_ptr<ISoftGpuMonitor>>(id, monitor));
    }
}

void SoftGpu::GetMonitor(uint32_t id, ISoftGpuMonitor** pMonitor)
{
    if (m_monitorMap.find(id) != m_monitorMap.end())
    {
        pMonitor = m_monitorMap[id].put();
    }
}

void SoftGpu::RemoveMonitor(uint32_t id)
{
    if (m_monitorMap.find(id) != m_monitorMap.end())
        m_monitorMap.erase(id);
}

//std::wstring SoftGpu::GetDisplayTargetName(UINT targetId)
//{
//    LUID adapterLUID = {};
//    DISPLAYCONFIG_TARGET_DEVICE_NAME targetName = {};
//
//    if (targetId == 0)
//    {
//        VERIFY_SUCCEEDED(m_softGpuAdapter->GetLuid(&adapterLUID));
//        VERIFY_SUCCEEDED(m_softGpuTarget1->GetId(&targetId));
//    }
//    else
//    {
//        VERIFY_SUCCEEDED(m_softGpuAdapter->GetLuid(&adapterLUID));
//        VERIFY_SUCCEEDED(m_softGpuTarget2->GetId(&targetId));
//    }
//
//    targetName.header.adapterId = adapterLUID;
//    targetName.header.id = targetId;
//    targetName.header.size = sizeof(targetName);
//    targetName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
//    THROW_IF_FAILED(DisplayConfigGetDeviceInfo(&targetName.header));
//
//    return std::wstring(targetName.monitorDevicePath);
//}
//
//LUID SoftGpu::GetAdapterLUID(UINT targetId)
//{
//    LUID adapterLUID = {};
//
//    if (targetId == 0)
//    {
//        VERIFY_SUCCEEDED(m_softGpuAdapter->GetLuid(&adapterLUID));
//    }
//    else
//    {
//        VERIFY_SUCCEEDED(m_softGpuAdapter->GetLuid(&adapterLUID));
//    }
//
//    return adapterLUID;
//}
//
//UINT SoftGpu::GetTargetId(UINT targetId)
//{
//    LUID adapterLUID = {};
//
//    if (targetId == 0)
//    {
//        VERIFY_SUCCEEDED(m_softGpuTarget1->GetId(&targetId));
//    }
//    else
//    {
//        VERIFY_SUCCEEDED(m_softGpuTarget2->GetId(&targetId));
//    }
//
//    return targetId;
//}