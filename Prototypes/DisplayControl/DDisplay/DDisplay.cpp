// DDisplay.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"

#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Devices.Display.Core.h>
#include <winrt/Windows.Graphics.DirectX.h>
#include <Windows.Devices.Display.Core.Interop.h>

namespace winrt
{
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Windows::Foundation::Collections;
    using namespace winrt::Windows::Devices::Display;
    using namespace winrt::Windows::Devices::Display::Core;
    using namespace winrt::Windows::Graphics;
    using namespace winrt::Windows::Graphics::DirectX;
}

using namespace DirectX;

static byte red = 0xff, green = 0xff, blue = 0xff;

#define MAX_HMD_TARGET_PER_ADAPTER 16
constexpr auto SURFACE_COUNT = 2;

typedef struct _D3DKMT_SOFTGPU_LUID_TARGET
{
    LUID AdapterLuid;   // In: Adapter LUID
    D3DDDI_VIDEO_PRESENT_TARGET_ID TargetId;   // In: TargetId
} D3DKMT_SOFTGPU_LUID_TARGET, * PD3DKMT_SOFTGPU_LUID_TARGET;

typedef struct _D3DKMT_SOFTGPU_HMD_CONTROL
{
    D3DKMT_SOFTGPU_LUID_TARGET LuidAndTargetList[MAX_HMD_TARGET_PER_ADAPTER];   // In: Array of LUID and TargetId pairs
    UINT LuidAndTargetListSize;   // In: Size of the populated LuidAndTargetList array
    BOOLEAN bEnableAsHMD; // In : Tells whether to enable or disable target as HMD
} D3DKMT_SOFTGPU_HMD_CONTROL, * PD3DKMT_SOFTGPU_HMD_CONTROL;

class MonitorControl
{
public:
    MonitorControl(LUID adapterId, UINT targetId, bool takeControl = true) :
        m_luid(adapterId), m_targetId(targetId), m_takeControl(takeControl)
    {
        Toggle();
    }

    ~MonitorControl()
    {
        if (IsValid())
        {
            Toggle(true);
        }
    }

    bool IsValid()
    {
        return SUCCEEDED(m_hr);
    }

private:
    void Toggle(bool reset = false)
    {
        D3DKMT_SOFTGPU_HMD_CONTROL SoftGpuHmdControl;
        SoftGpuHmdControl.LuidAndTargetList[0].AdapterLuid = m_luid;
        SoftGpuHmdControl.LuidAndTargetList[0].TargetId = m_targetId;
        SoftGpuHmdControl.LuidAndTargetListSize = 1;
        SoftGpuHmdControl.bEnableAsHMD = (reset) ? !m_takeControl : !!m_takeControl;

        // Escape for controling HMD from SoftGpu.
        D3DKMT_ESCAPE sEsc = {};
        sEsc.Type = D3DKMT_ESCAPE_SOFTGPU_ENABLE_DISABLE_HMD;
        sEsc.pPrivateDriverData = &SoftGpuHmdControl;
        sEsc.PrivateDriverDataSize = sizeof(SoftGpuHmdControl);

        m_hr = D3DKMTEscape(&sEsc);
    }

    HRESULT m_hr = S_OK;
    LUID m_luid;
    UINT m_targetId;
    bool m_takeControl;
};

struct RenderParam
{
    RenderParam(
        const std::atomic_bool& shouldTerminate) :
        shouldTerminate(shouldTerminate)
    {}

    const std::atomic_bool& shouldTerminate;
    winrt::DisplayDevice device{ nullptr };
    winrt::DisplayTarget target{ nullptr };
    winrt::DisplayPath path{ nullptr };
};

LUID LuidFromAdapterId(winrt::Windows::Graphics::DisplayAdapterId id)
{
    return { id.LowPart, id.HighPart };
}

class D3D11Renderer
{
    winrt::com_ptr<ID3D11Device5> d3dDevice;
    winrt::com_ptr<ID3D11DeviceContext> d3dContext;

    std::array<winrt::com_ptr<ID3D11Texture2D>, SURFACE_COUNT> d3dSurfaces;
    std::array<winrt::com_ptr<ID3D11RenderTargetView>, SURFACE_COUNT> d3dRenderTargets;
    winrt::com_ptr<ID3D11Fence> d3dFence;
    UINT64 fenceValue = 0;
    int frameCount = 0;

public:

    void Create(const winrt::DisplayAdapter& adapter)
    {
        winrt::com_ptr<IDXGIFactory6> factory;
        factory.capture(&CreateDXGIFactory2, 0);

        // Find the GPU that the target is connected to
        winrt::com_ptr<IDXGIAdapter4> dxgiAdapter;
        dxgiAdapter.capture(factory, &IDXGIFactory6::EnumAdapterByLuid, LuidFromAdapterId(adapter.Id()));

        // Create the D3D device and context from the adapter
        D3D_FEATURE_LEVEL featureLevel;
        winrt::com_ptr<ID3D11Device> device;
        winrt::check_hresult(D3D11CreateDevice(dxgiAdapter.get(), D3D_DRIVER_TYPE_UNKNOWN, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, device.put(), &featureLevel, d3dContext.put()));
        d3dDevice = device.as<ID3D11Device5>();

        // Create a fence for signalling when rendering work finishes
        d3dFence.capture(d3dDevice, &ID3D11Device5::CreateFence, 0, D3D11_FENCE_FLAG_SHARED);
    }

    void OpenSurfaces(const winrt::DisplayDevice& device, std::array<winrt::DisplaySurface, SURFACE_COUNT>& surfaces)
    {
        auto deviceInterop = device.as<IDisplayDeviceInterop>();

        for (int surfaceIndex = 0; surfaceIndex < SURFACE_COUNT; surfaceIndex++)
        {
            auto surfaceRaw = surfaces[surfaceIndex].as<::IInspectable>();

            // Share the DisplaySurface across devices using a handle
            winrt::handle surfaceHandle;
            winrt::check_hresult(deviceInterop->CreateSharedHandle(surfaceRaw.get(), nullptr, GENERIC_ALL, nullptr, surfaceHandle.put()));

            // Call OpenSharedResource1 on the D3D device to get the ID3D11Texture2D
            d3dSurfaces[surfaceIndex].capture(d3dDevice, &ID3D11Device5::OpenSharedResource1, surfaceHandle.get());

            D3D11_TEXTURE2D_DESC surfaceDesc = {};
            d3dSurfaces[surfaceIndex]->GetDesc(&surfaceDesc);

            D3D11_RENDER_TARGET_VIEW_DESC viewDesc = {};
            viewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
            viewDesc.Texture2D.MipSlice = 0;
            viewDesc.Format = surfaceDesc.Format;

            // Create a render target view for the surface
            winrt::check_hresult(d3dDevice->CreateRenderTargetView(d3dSurfaces[surfaceIndex].get(), &viewDesc, d3dRenderTargets[surfaceIndex].put()));
        }
    }

    winrt::DisplayFence GetFence(const winrt::DisplayDevice& device)
    {
        auto deviceInterop = device.as<IDisplayDeviceInterop>();

        // Share the ID3D11Fence across devices using a handle
        winrt::handle fenceHandle;
        winrt::check_hresult(d3dFence->CreateSharedHandle(nullptr, GENERIC_ALL, nullptr, fenceHandle.put()));

        // Call OpenSharedHandle on the DisplayDevice to get a DisplayFence
        winrt::com_ptr<::IInspectable> displayFence;
        displayFence.capture(deviceInterop, &IDisplayDeviceInterop::OpenSharedHandle, fenceHandle.get());

        return displayFence.as<winrt::DisplayFence>();
    }

    UINT64 RenderAndGetFenceValue(int surfaceIndex)
    {
        // TODO: Perform rendering here with D3D11

        // For the sample, we simply render a color pattern using a frame counter. This code is not interesting.
        {
            frameCount++;
            float clearColor[4] = { (float)red / 255.f, (float)green / 255.f, (float)blue / 255.f, 1 };
            d3dContext->ClearRenderTargetView(d3dRenderTargets[surfaceIndex].get(), clearColor);
        }

        auto context4 = d3dContext.as<ID3D11DeviceContext4>();
        context4->Signal(d3dFence.get(), ++fenceValue);

        return fenceValue;
    }
};

void RenderThread(RenderParam& params)
{
    // It's not necessary to call init_apartment on every thread, but it needs to be called at least once before using WinRT
    winrt::init_apartment();

    D3D11Renderer renderer;
    renderer.Create(params.target.Adapter());

    // Create a display source, which identifies where to render
    winrt::DisplaySource source = params.device.CreateScanoutSource(params.target);

    // Create a task pool for queueing presents
    winrt::DisplayTaskPool taskPool = params.device.CreateTaskPool();

    winrt::SizeInt32 sourceResolution = params.path.SourceResolution().Value();
    winrt::Direct3D11::Direct3DMultisampleDescription multisampleDesc = {};
    multisampleDesc.Count = 1;

    // Create a surface format description for the primaries
    winrt::DisplayPrimaryDescription primaryDesc{
        static_cast<uint32_t>(sourceResolution.Width), static_cast<uint32_t>(sourceResolution.Height),
        params.path.SourcePixelFormat(), winrt::DirectXColorSpace::RgbFullG22NoneP709,
        false,
        multisampleDesc };

    std::array<winrt::DisplaySurface, SURFACE_COUNT> primaries = { nullptr, nullptr };
    std::array<winrt::DisplayScanout, SURFACE_COUNT> scanouts = { nullptr, nullptr };

    for (int surfaceIndex = 0; surfaceIndex < SURFACE_COUNT; surfaceIndex++)
    {
        primaries[surfaceIndex] = params.device.CreatePrimary(params.target, primaryDesc);
        scanouts[surfaceIndex] = params.device.CreateSimpleScanout(source, primaries[surfaceIndex], 0, 1);
    }

    renderer.OpenSurfaces(params.device, primaries);

    // Get a fence to wait for render work to complete
    winrt::DisplayFence fence = renderer.GetFence(params.device);

    // Render and present until termination is signalled
    int surfaceIndex = 0;
    while (!params.shouldTerminate)
    {
        UINT64 fenceValue = renderer.RenderAndGetFenceValue(surfaceIndex);

        winrt::DisplayTask task = taskPool.CreateTask();
        task.SetScanout(scanouts[surfaceIndex]);
        task.SetWait(fence, fenceValue);

        taskPool.ExecuteTask(task);

        params.device.WaitForVBlank(source);

        surfaceIndex++;
        if (surfaceIndex >= SURFACE_COUNT)
        {
            surfaceIndex = 0;
        }
    }
}

class Display
{
public:
    Display()
    {
        winrt::init_apartment();

        m_manager = std::make_shared<winrt::DisplayManager>(winrt::DisplayManager::Create(winrt::DisplayManagerOptions::None));
    }

    bool SelectAndProcess()
    {
        auto targets = m_manager->GetCurrentTargets();

        auto testTargets = winrt::single_threaded_map<int32_t, winrt::DisplayTarget>();

        int32_t index = 0;
        for (auto&& target : targets)
        {
            if (target.IsConnected())
            {
                testTargets.Insert(index++, target);
            }
        }

        std::wcout << L"Connected Displays: " << std::endl;
        for (auto&& target : testTargets)
        {
            std::wcout << L"\t" << target.Key() << L". " << target.Value().StableMonitorId().c_str() << std::endl;
        }

        int32_t selection = -1;
        while (1)
        {
            std::wcout << L"\nChoose a display number: ";
            std::cin >> selection;
            getchar(); // eat the newline

            if (testTargets.HasKey(selection)) break;
            else
            {
                std::wcout << L"\n\t\tInvalid choice" << std::endl;
            }
        }

        m_target = std::make_shared<winrt::DisplayTarget>(testTargets.Lookup(selection));

        LUID adapterLUID = { 0 };
        adapterLUID.HighPart = m_target->Adapter().Id().HighPart;
        adapterLUID.LowPart = m_target->Adapter().Id().LowPart;

        // Remove the selected target from composition
        auto monitorReservation = MonitorControl(adapterLUID, m_target->AdapterRelativeId());

        RefreshTargetIfStale();

        auto targetList = winrt::single_threaded_vector<winrt::DisplayTarget>();
        targetList.Append(*m_target);
        auto stateResult = m_manager->TryAcquireTargetsAndCreateEmptyState(targetList);

        for (UINT loop = 0; loop < 10 && stateResult.ErrorCode() != winrt::DisplayManagerResult::Success; loop++)
        {
            std::wcout << L"Failed to acquire display... ";
            Sleep(1000);
            RefreshTargetIfStale();
            std::wcout << L"Retrying" << std::endl;

            auto targetList = winrt::single_threaded_vector<winrt::DisplayTarget>();
            targetList.Append(*m_target);
            auto stateResult = m_manager->TryAcquireTargetsAndCreateEmptyState(targetList);
        }

        if (stateResult.ErrorCode() != winrt::DisplayManagerResult::Success)
        {
            std::wcout << L"Acquiring display failed, DisplayManagerResult " << (int32_t)stateResult.ErrorCode() << std::endl;
            return false;
        }

        m_state = std::make_shared<winrt::DisplayState>(stateResult.State());

        auto path = m_state->ConnectTarget(*m_target);

        path.IsInterlaced(false);
        path.Scaling(winrt::DisplayPathScaling::Identity);

        path.SourcePixelFormat(winrt::DirectXPixelFormat::R8G8B8A8UIntNormalized);

        auto modes = path.FindModes(winrt::DisplayModeQueryOptions::OnlyPreferredResolution);

        winrt::DisplayModeInfo bestMode{ nullptr };
        double bestModeDiff = INFINITY;
        for (auto&& mode : modes)
        {
            auto vSync = mode.PresentationRate().VerticalSyncRate;
            double vSyncDouble = (double)vSync.Numerator / vSync.Denominator;

            double modeDiff = abs(vSyncDouble - 60);
            if (modeDiff < bestModeDiff)
            {
                bestMode = mode;
                bestModeDiff = modeDiff;
            }
        }

        if (!bestMode)
        {
            std::wcout << L"Failed to find a valid mode" << std::endl;
            return false;
        }

        // Set the properties on the path
        path.ApplyPropertiesFromMode(bestMode);

        auto result = m_state->TryApply(winrt::DisplayStateApplyOptions::None);

        if (FAILED(result.ExtendedErrorCode()))
        {
            std::wcout << L"Failed to apply the chosen mode" << std::endl;
            return false;
        }

        std::atomic_bool stopRender = false;

        std::unique_ptr<RenderParam> params = std::make_unique<RenderParam>(stopRender);

        params->device = m_manager->CreateDisplayDevice(m_target->Adapter());
        params->target = *m_target;
        params->path = m_state->GetPathForTarget(*m_target);

        std::thread renderThread([params = std::move(params)]()
        {
            RenderThread(*params);
        });

        do {
            std::wcout << L'\n' << L"Press a key to continue...";
        } while (std::wcin.get() != L'\n');

        stopRender = true;

        renderThread.join();

        return true;
    }

    std::shared_ptr<winrt::DisplayManager> GetManager() { return m_manager; }
    std::shared_ptr<winrt::DisplayTarget> GetTarget() { return m_target; }

private:
    std::shared_ptr<winrt::DisplayManager> m_manager;
    std::shared_ptr<winrt::DisplayTarget> m_target;
    std::unique_ptr<MonitorControl> m_monitorControl;

    std::shared_ptr<winrt::DisplayState> m_state;

    void RefreshTargetIfStale()
    {
        if (m_target && m_target->IsStale())
        {
            auto targets = m_manager->GetCurrentTargets();

            for (auto&& target : targets)
            {
                if (target.IsConnected() && target.IsSame(*m_target))
                {
                    m_target = std::make_shared<winrt::DisplayTarget>(target);
                    break;
                }
            }
        }
    }
};

int main(int argc, char* argv[])
{
    if (argc == 4)
    {
        red = (byte)atoi(argv[1]);
        green = (byte)atoi(argv[2]);
        blue = (byte)atoi(argv[3]);
    }
    auto display = Display();

    if (!display.SelectAndProcess())
    {
        return -1;
    }

    return 0;
}