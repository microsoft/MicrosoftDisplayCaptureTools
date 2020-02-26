#include "pch.h"
#include "TestEnvironment.h"
#include "TestEnvironment.g.cpp"

#define MAX_HMD_TARGET_PER_ADAPTER 16
#define EDID_V1_BLOCK_SIZE 128

typedef struct _D3DKMT_HMD_DISPLAY_ENUM
{
    LUID AdapterLuid;   // In: Adapter LUID
    UINT TargetIdListSize;   // Out: Size of the populated TargetIdList array
    D3DDDI_VIDEO_PRESENT_TARGET_ID TargetIdList[MAX_HMD_TARGET_PER_ADAPTER];   // Out: Array for TargetIds
} D3DKMT_HMD_DISPLAY_ENUM, * PD3DKMT_HMD_DISPLAY_ENUM;

typedef enum
{
    D3DKMT_HMD_SET_DISPLAY_ON = 0,
    D3DKMT_HMD_SET_DISPLAY_OFF = 1,
    D3DKMT_HMD_GET_DISPLAY_STATE = 2,
} D3DKMT_HMD_DISPLAY_REQUEST;

typedef struct _D3DKMT_HMD_DISPLAY_CONTROL
{
    LUID AdapterLuid;   // In: Adapter LUID
    D3DDDI_VIDEO_PRESENT_TARGET_ID TargetId;    // In: Target ID for a unique HMD
    D3DKMT_HMD_DISPLAY_REQUEST HmdDisplayRequest;   // In: HMD Request type
    BOOLEAN bIsHmdDisplayOn;    // Out : HMD's current On/Off state
} D3DKMT_HMD_DISPLAY_CONTROL, * PD3DKMT_HMD_DISPLAY_CONTROL;

typedef struct _D3DKMT_HMD_GET_EDID
{
    LUID AdapterLuid;   // In: Adapter LUID
    D3DDDI_VIDEO_PRESENT_TARGET_ID TargetId;    // In: Target ID for a unique HMD
    BYTE EdidBaseBlock[EDID_V1_BLOCK_SIZE];   // Out: EDID base block
} D3DKMT_HMD_GET_EDID, * PD3DKMT_HMD_GET_EDID;

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

#define D3DKMT_ESCAPE_SOFTGPU_ENABLE_DISABLE_HMD ((D3DKMT_ESCAPETYPE)27)


namespace winrt
{
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Windows::Devices::Display;
    using namespace winrt::Windows::Devices::Display::Core;
    using namespace winrt::Windows::Graphics;
    using namespace winrt::Windows::Graphics::DirectX;
    using namespace winrt::Windows::Foundation::Collections;
}

namespace winrt::TestFramework::implementation
{
    HRESULT ToggleDisplay(DisplayAdapterId adapter, D3DDDI_VIDEO_PRESENT_TARGET_ID target, bool takeControl)
    {
        D3DKMT_SOFTGPU_HMD_CONTROL SoftGpuHmdControl;

        SoftGpuHmdControl.LuidAndTargetList[0].AdapterLuid.HighPart = adapter.HighPart;
        SoftGpuHmdControl.LuidAndTargetList[0].AdapterLuid.LowPart = adapter.LowPart;
        SoftGpuHmdControl.LuidAndTargetList[0].TargetId = target;
        SoftGpuHmdControl.LuidAndTargetListSize = 1;
        SoftGpuHmdControl.bEnableAsHMD = !!takeControl;

        D3DKMT_ESCAPE sEsc = {};
        sEsc.Type = D3DKMT_ESCAPE_SOFTGPU_ENABLE_DISABLE_HMD;
        sEsc.pPrivateDriverData = &SoftGpuHmdControl;
        sEsc.PrivateDriverDataSize = sizeof(SoftGpuHmdControl);
        return HRESULT_FROM_NT(::D3DKMTEscape(&sEsc));
    }

    // Parameters passed to each render thread
    struct RenderParam
    {
        RenderParam(const std::atomic_bool& shouldTerminate) :
            shouldTerminate(shouldTerminate)
        {}

        const std::atomic_bool& shouldTerminate;
        winrt::DisplayDevice device{ nullptr };
        winrt::DisplayTarget target{ nullptr };
        winrt::DisplayPath path{ nullptr };
    };

    ::LUID LuidFromAdapterId(winrt::Windows::Graphics::DisplayAdapterId id)
    {
        return { id.LowPart, id.HighPart };
    }

    const int SurfaceCount = 2;

    class D3D11Renderer
    {
        winrt::com_ptr<ID3D11Device5> d3dDevice;
        winrt::com_ptr<ID3D11DeviceContext> d3dContext;

        std::array<winrt::com_ptr<ID3D11Texture2D>, SurfaceCount> d3dSurfaces;
        std::array<winrt::com_ptr<ID3D11RenderTargetView>, SurfaceCount> d3dRenderTargets;
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

        void OpenSurfaces(const winrt::DisplayDevice& device, std::array<winrt::DisplaySurface, SurfaceCount>& surfaces)
        {
            auto deviceInterop = device.as<IDisplayDeviceInterop>();

            for (int surfaceIndex = 0; surfaceIndex < SurfaceCount; surfaceIndex++)
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
                float amount = (float)abs(sin((float)frameCount / 30 * 3.141592));
                float clearColor[4] = { amount * ((frameCount / 30) % 3 == 0), amount * ((frameCount / 30) % 3 == 1), amount * ((frameCount / 30) % 3 == 2), 1 };
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

        std::array<winrt::DisplaySurface, SurfaceCount> primaries = { nullptr, nullptr };
        std::array<winrt::DisplayScanout, SurfaceCount> scanouts = { nullptr, nullptr };

        for (int surfaceIndex = 0; surfaceIndex < SurfaceCount; surfaceIndex++)
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
            if (surfaceIndex >= SurfaceCount)
            {
                surfaceIndex = 0;
            }
        }
    }

    TestEnvironment::TestEnvironment(array_view<Windows::Devices::Display::Core::DisplayTarget const> displayTargets)
    {
        auto displayManager = winrt::DisplayManager::Create(winrt::DisplayManagerOptions::None);
        auto myTargets = winrt::single_threaded_vector<winrt::DisplayTarget>();

        for (auto&& target : displayTargets)
        {
            if (target.IsConnected())
            {
                // Set the display as specialized
                check_hresult(ToggleDisplay(target.Adapter().Id(), target.AdapterRelativeId(), true));

                myTargets.Append(target);
                _displayTargets.push_back(target);
            }
        }

        auto stateResult = displayManager.TryAcquireTargetsAndCreateEmptyState(myTargets);
   
        // Make sure to re-add the displays in the event we can't take control of the display for some reason.
        if (FAILED(stateResult.ExtendedErrorCode()))
        {
            CleanDisplays();
            check_hresult(stateResult.ExtendedErrorCode());
        }

        auto state = stateResult.State();

        for (auto&& target : myTargets)
        {
            winrt::DisplayPath path = state.ConnectTarget(target);

            // Set some values that we know we want
            path.IsInterlaced(false);
            path.Scaling(winrt::DisplayPathScaling::Identity);

            // We only look at BGRA 8888 modes in this example
            path.SourcePixelFormat(winrt::DirectXPixelFormat::B8G8R8A8UIntNormalized);

            // Get a list of modes for only the preferred resolution
            winrt::IVectorView<winrt::DisplayModeInfo> modes = path.FindModes(winrt::DisplayModeQueryOptions::OnlyPreferredResolution);

            // Find e.g. the mode with a refresh rate closest to 60 Hz
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
                // Failed to find a mode
                std::cout << "Failed to find a valid mode" << std::endl;
                throw_hresult(hresult_access_denied().code());
            }

            // Set the properties on the path
            path.ApplyPropertiesFromMode(bestMode);
        }

        // Now that we've decided on modes to use for all of the targets, apply all the modes in one-shot
        auto applyResult = state.TryApply(winrt::DisplayStateApplyOptions::None);
        check_hresult(applyResult.ExtendedErrorCode());

        // Re-read the current state to see the final state that was applied (with all properties)
        stateResult = displayManager.TryAcquireTargetsAndReadCurrentState(myTargets);
        check_hresult(stateResult.ExtendedErrorCode());
        state = stateResult.State();

        std::atomic_bool shouldCancelRenderThreads;
        std::vector<std::thread> renderThreads;

        for (auto&& target : myTargets)
        {
            std::unique_ptr<RenderParam> params = std::make_unique<RenderParam>(shouldCancelRenderThreads);

            // Create a device to present with
            params->device = displayManager.CreateDisplayDevice(target.Adapter());

            params->target = target;
            params->path = state.GetPathForTarget(target);

            std::thread renderThread([params = std::move(params)]()
            {
                RenderThread(*params);
            });

            renderThreads.push_back(std::move(renderThread));
        }

        // Render for 10 seconds
        Sleep(10000);

        // Trigger all render threads to terminate
        shouldCancelRenderThreads = true;

        // Wait for all threads to complete
        for (auto&& thread : renderThreads)
        {
            thread.join();
        }
    }

    void TestEnvironment::Run()
    {
        throw hresult_not_implemented();
    }

    TestFramework::OSOverrides TestEnvironment::GetOSOverrides()
    {
        throw hresult_not_implemented();
    }

    TestEnvironment::~TestEnvironment()
    {
        CleanDisplays();
    }
    void TestEnvironment::CleanDisplays()
    {
        for (auto target : _displayTargets)
        {
            ToggleDisplay(target.Adapter().Id(), target.AdapterRelativeId(), false);
        }
    }
}
