#include "pch.h"
#include "DisplayEngine.h"
#include "DisplayEngine.g.cpp"
#include "DisplayEngineFactory.g.cpp"

import MonitorControl;

using namespace winrt::MicrosoftDisplayCaptureTools::Framework::Helpers;
namespace winrt {
    // Standard WinRT inclusions
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Windows::Foundation::Collections;

    // Windows data streams
    using namespace winrt::Windows::Storage::Streams;

    // JSON Parser
    using namespace winrt::Windows::Data::Json;

    // Direct Display
    using namespace winrt::Windows::Devices::Display;
    using namespace winrt::Windows::Devices::Display::Core;

    // DirectX
    using namespace winrt::Windows::Graphics;
    using namespace winrt::Windows::Graphics::DirectX;
    using namespace winrt::Windows::Graphics::Imaging;

    // Hardware HLK project
    using namespace winrt::MicrosoftDisplayCaptureTools::Display;
    using namespace winrt::MicrosoftDisplayCaptureTools::Framework;
    using namespace winrt::MicrosoftDisplayCaptureTools::Libraries;
} // namespace winrt

// Disable 'unreferenced formal parameter' errors
#pragma warning(push)
#pragma warning(disable : 4100)

namespace winrt::BasicDisplayControl::implementation {
    winrt::IDisplayEngine DisplayEngineFactory::CreateDisplayEngine()
    {
        return winrt::make<DisplayEngine>();
    }

    DisplayEngine::DisplayEngine()
    {
        m_displayManager = winrt::DisplayManager::Create(winrt::DisplayManagerOptions::None);
    }

    DisplayEngine::~DisplayEngine()
    {
    }

    void DisplayEngine::SetConfigData(IJsonValue data)
    {
    }

    IDisplayOutput DisplayEngine::InitializeOutput(winrt::DisplayTarget const& target)
    {
        try
        {
            auto output = winrt::make<DisplayOutput>(target, m_displayManager);

            return output;
        }
        catch (...)
        {
            Logger().LogError(L"Unable to initialize display output.");
            return nullptr;
        }
    }

    IDisplayOutput DisplayEngine::InitializeOutput(winrt::hstring target)
    {
        // Translate the target string to a DisplayTarget and call 'InitializeForDisplayTarget'
        winrt::DisplayTarget chosenTarget{nullptr};
        auto allDisplayTargets = m_displayManager.GetCurrentTargets();
        for (auto&& displayTarget : allDisplayTargets)
        {
            if (displayTarget.StableMonitorId() == target)
            {
                chosenTarget = displayTarget;
                break;
            }
        }

        if (!chosenTarget)
        {
            // The chosen target was not found - was the config file generated for this machine?
            Logger().LogError(L"The chosen target was not found - was the configuration used generated for this machine?");
            throw winrt::hresult_invalid_argument();
        }

        return InitializeOutput(chosenTarget);
    }

    // DisplayOutput
    DisplayOutput::DisplayOutput(DisplayTarget const& target, DisplayManager const& manager) :
        m_displayTarget(target), m_displayManager(manager)
    {
        // Refresh the display target
        RefreshTarget();

        if (!target)
        {
            // The chosen target was not found - was the config file generated for this machine?
            Logger().LogError(L"Attempted to initialize with a null display target.");
            throw winrt::hresult_invalid_argument();
        }

        // If needed, remove the targeted display from composition.
        if (m_displayTarget.UsageKind() != winrt::DisplayMonitorUsageKind::SpecialPurpose)
        {
            m_monitorControl = std::make_unique<MonitorUtilities::MonitorControl>(
                MonitorUtilities::LuidFromAdapterId(m_displayTarget.Adapter().Id()), m_displayTarget.AdapterRelativeId());

            // If we change the UsageKind for the displayTarget, make sure to go refresh the target
            DisplayTarget refreshedTarget = nullptr;
            for (auto&& currTarget : m_displayManager.GetCurrentTargets())
            {
                if (m_displayTarget.IsSame(currTarget))
                {
                    refreshedTarget = currTarget;
                    break;
                }
            }

            if (!refreshedTarget)
            {
                Logger().LogError(L"Unable to locate the target display after marking it as 'Specialized'");
                throw winrt::hresult_changed_state();
            }

            m_displayTarget = refreshedTarget;
        }

        ConnectTarget();

        // Instantiate properties object
        m_properties = winrt::make_self<DisplayEngineProperties>();

        // This DisplayEngine implementation only supports 1 plane
        auto planeProperties = winrt::make_self<DisplayEnginePlaneProperties>();
        m_properties->m_planeProperties.push_back(planeProperties);

        // Mark the base plane as active
        m_properties->m_planeProperties[0]->Active(true);
    }

    DisplayOutput::~DisplayOutput()
    {
        if (m_displayTarget && m_displayState)
        {
            m_displayState.DisconnectTarget(m_displayTarget);
        }
    }

    winrt::DisplayTarget DisplayOutput::Target()
    {
        return m_displayTarget;
    }
    winrt::IDisplayEngineProperties DisplayOutput::GetProperties()
    {
        return *m_properties;
    }

    winrt::IClosable DisplayOutput::StartRender()
    {
        // Re-connect the target
        ConnectTarget();

        // Create an object to stop the rendering when destroyed/closed.
        auto renderWatchdog = make_self<RenderWatchdog>(this);

        // Start the rendering process - first this will determine eligible modes and then it will spin off a thread to run
        // a render loop. This function won't return until the thread has started.
        if (PrepareRender())
        {
            return renderWatchdog.as<IClosable>();
        }

        return nullptr;
    }

    event_token DisplayOutput::DisplaySetupCallback(Windows::Foundation::EventHandler<IDisplaySetupToolArgs> const& handler)
    {
        return m_displaySetupCallback.add(handler);
    }

    void DisplayOutput::DisplaySetupCallback(event_token const& token) noexcept
    {
        m_displaySetupCallback.remove(token);
    }

    event_token DisplayOutput::RenderSetupCallback(Windows::Foundation::EventHandler<IRenderSetupToolArgs> const& handler)
    {
        return m_renderSetupCallback.add(handler);
    }

    void DisplayOutput::RenderSetupCallback(event_token const& token) noexcept
    {
        m_renderSetupCallback.remove(token);
    }

    event_token DisplayOutput::RenderLoopCallback(Windows::Foundation::EventHandler<IRenderingToolArgs> const& handler)
    {
        return m_renderLoopCallback.add(handler);
    }

    void DisplayOutput::RenderLoopCallback(event_token const& token) noexcept
    {
        m_renderLoopCallback.remove(token);
    }

    void DisplayOutput::RefreshTarget()
    {
        if (m_displayManager && m_displayTarget && m_displayTarget.IsStale())
        {
            auto currentTargets = m_displayManager.GetCurrentTargets();
            for (auto&& currentTarget : currentTargets)
            {
                if (currentTarget.IsConnected() && currentTarget.IsSame(m_displayTarget))
                {
                    m_displayTarget = currentTarget;
                    return;
                }
            }

            // The selected target is no longer showing up as current or connected!
            Logger().LogError(L"The selected target can no longer be found. Selected Target: " + m_displayTarget.StableMonitorId());
            throw winrt::hresult_changed_state();
        }
    }

    void DisplayOutput::ConnectTarget()
    {
        // Disconnect if already connected
        if (m_displayPath && m_displayState && m_displayTarget)
        {
            m_displayState.DisconnectTarget(m_displayTarget);
            m_displayState = nullptr;
            m_displayPath = nullptr;
        }

        auto targetList = winrt::single_threaded_vector<winrt::DisplayTarget>();
        targetList.Append(m_displayTarget);
        auto result = m_displayManager.TryAcquireTargetsAndCreateEmptyState(targetList);

        // TryAcquireTargetsAndCreateEmptyState is a very flaky call - so try it again a few times if it fails
        const uint32_t retries = 20;
        const uint32_t retryDelay = 500;
        for (uint32_t i = 0; i < retries && result.ErrorCode() != winrt::DisplayManagerResult::Success; i++, Sleep(retryDelay))
        {
            RefreshTarget();

            targetList.Clear();
            targetList.Append(m_displayTarget);
            result = m_displayManager.TryAcquireTargetsAndCreateEmptyState(targetList);
        }

        if (result.ErrorCode() != winrt::DisplayManagerResult::Success)
        {
            // We failed to acquire control of the target.
            Logger().LogError(
                winrt::hstring(L"Failed to acquire control of the target. Error: ") + winrt::to_hstring(result.ExtendedErrorCode()));
            throw result.ExtendedErrorCode();
        }

        m_displayState = result.State();
        m_displayPath = m_displayState.ConnectTarget(m_displayTarget);
        m_displayDevice = m_displayManager.CreateDisplayDevice(m_displayTarget.Adapter());
    }

    // Rendering
    void DisplayOutput::StopRender()
    {
        Logger().LogNote(L"Stopping Renderer Thread");

        if (m_valid)
        {
            m_valid = false;

            if (m_renderThread.joinable())
            {
                m_renderThread.join();
            }
        }
    }

    bool DisplayOutput::RefreshMode()
    {
        if (m_properties->RequeryMode())
        {
            auto modeList = m_displayPath.FindModes(winrt::DisplayModeQueryOptions::None);

            // TODO: remove modeListVec - it's just for experimenting with a debugger.
            std::vector<DisplayModeInfo> modeListVec;

            for (auto&& mode : modeList)
            {
                if (mode.SourceResolution().Width == 3840 && 5 > fabs(
                                                                     ((double)mode.PresentationRate().VerticalSyncRate.Numerator /
                                                                      (double)mode.PresentationRate().VerticalSyncRate.Denominator) -
                                                                     60))
                    modeListVec.push_back(mode);

                // Check to see if this mode is acceptable
                if (m_displaySetupCallback)
                {
                    auto displaySetupArgs =
                        winrt::make_self<DisplaySetupToolArgs>(m_properties.as<IDisplayEngineProperties>(), mode);

                    m_displaySetupCallback(*this, displaySetupArgs.as<IDisplaySetupToolArgs>());

                    if (displaySetupArgs->Compatible())
                    {
                        // we have a mode matching the requirements.
                        Logger().LogNote(L"Found an acceptable mode.");

                        m_properties->ActiveMode(mode);
                        return true;
                    }
                }
            }

            // No mode fit the set tools - this _may_ indicate an error, but it may also just indicate that we are attempting
            // to auto-configure. So log a warning instead of an error to assist the user. If this is an actual error case, 
            // later operations will fail from this case and log the error.
            Logger().LogWarning(L"No display mode fit the selected options");
            return false;
        }

        return true;
    }

    bool DisplayOutput::PrepareRender()
    {
        Logger().LogNote(L"Preparing Renderer Thread");

        m_presenting = false;

        if (!RefreshMode())
        {
            return false;
        }

        m_displayPath.ApplyPropertiesFromMode(m_properties->ActiveMode());
        auto result = m_displayState.TryApply(winrt::DisplayStateApplyOptions::FailIfStateChanged);

        if (result.Status() != DisplayStateOperationStatus::Success)
        {
            Logger().LogError(L"Applying modes to the display failed.");
        }

        m_renderThread = std::thread(&DisplayOutput::RenderLoop, this);

        // don't return until the render thread has started presenting
        while (!m_presenting)
        {
            std::this_thread::yield();
        }

        return true;
    }

    void DisplayOutput::RenderLoop()
    {
        winrt::init_apartment();

        Logger().LogNote(L"Starting Render");

        uint64_t frameCounter = 0;

        // This DisplayEngine only supports a single plane, so directly use the base plane's properties here.
        auto basePlaneProperties = m_properties->GetPlaneProperties()[0].as<DisplayEnginePlaneProperties>();

        // Initialize D3D objects
        winrt::com_ptr<IDXGIFactory6> dxgiFactory;
        winrt::com_ptr<ID3D11Device5> d3dDevice;
        winrt::com_ptr<ID3D11DeviceContext> d3dContext;
        winrt::com_ptr<ID3D11Fence> d3dFence;
        uint64_t d3dFenceValue = 0;

        dxgiFactory.capture(&CreateDXGIFactory2, 0);

        winrt::com_ptr<IDXGIAdapter4> dxgiAdapter;
        dxgiAdapter.capture(
            dxgiFactory, &IDXGIFactory7::EnumAdapterByLuid, MonitorUtilities::LuidFromAdapterId(m_displayTarget.Adapter().Id()));

        D3D_FEATURE_LEVEL featureLevel;
        winrt::com_ptr<ID3D11Device> device;
        winrt::check_hresult(D3D11CreateDevice(
            dxgiAdapter.get(),
            D3D_DRIVER_TYPE_UNKNOWN,
            nullptr,
            D3D11_CREATE_DEVICE_BGRA_SUPPORT,
            nullptr,
            0,
            D3D11_SDK_VERSION,
            device.put(),
            &featureLevel,
            d3dContext.put()));
        d3dDevice = device.as<ID3D11Device5>();

        d3dFence.capture(d3dDevice, &ID3D11Device5::CreateFence, 0, D3D11_FENCE_FLAG_SHARED);

        // Initialize DDisplay sources and tasks
        auto taskPool = m_displayDevice.CreateTaskPool();
        auto source = m_displayDevice.CreateScanoutSource(m_displayTarget);

        winrt::SizeInt32 sourceResolution = m_displayPath.SourceResolution().Value();
        winrt::Direct3D11::Direct3DMultisampleDescription multisampleDesc = {};
        multisampleDesc.Count = 1;

        // Create a surface format description for the primaries
        winrt::DisplayPrimaryDescription primaryDesc{
            static_cast<uint32_t>(sourceResolution.Width),
            static_cast<uint32_t>(sourceResolution.Height),
            m_displayPath.SourcePixelFormat(),
            winrt::DirectXColorSpace::RgbFullG22NoneP709,
            false,
            multisampleDesc};

        winrt::DisplaySurface primarySurface{nullptr};
        winrt::DisplayScanout primaryScanout{nullptr};

        primarySurface = m_displayDevice.CreatePrimary(m_displayTarget, primaryDesc);
        primaryScanout = m_displayDevice.CreateSimpleScanout(source, primarySurface, 0, 1);

        auto interopDevice = m_displayDevice.as<IDisplayDeviceInterop>();
        auto rawSurface = primarySurface.as<::IInspectable>();

        winrt::handle primarySurfaceHandle;
        winrt::check_hresult(
            interopDevice->CreateSharedHandle(rawSurface.get(), nullptr, GENERIC_ALL, nullptr, primarySurfaceHandle.put()));

        winrt::com_ptr<ID3D11RenderTargetView> d3dRenderTarget;

        {
            winrt::com_ptr<ID3D11Texture2D> d3dSurface;
            d3dSurface.capture(d3dDevice, &ID3D11Device5::OpenSharedResource1, primarySurfaceHandle.get());
            basePlaneProperties->SetPlaneTexture(d3dSurface.get());
        }

        winrt::com_ptr<ID3D11Texture2D> basePlaneSurface;
        winrt::check_hresult(basePlaneProperties->GetPlaneTexture(basePlaneSurface.put()));

        D3D11_TEXTURE2D_DESC d3dTextureDesc{};
        basePlaneSurface->GetDesc(&d3dTextureDesc);

        D3D11_RENDER_TARGET_VIEW_DESC d3dRenderTargetViewDesc{};
        d3dRenderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        d3dRenderTargetViewDesc.Texture2D.MipSlice = 0;
        d3dRenderTargetViewDesc.Format = d3dTextureDesc.Format;

        // Create the render target view
        winrt::check_hresult(d3dDevice->CreateRenderTargetView(basePlaneSurface.get(), &d3dRenderTargetViewDesc, d3dRenderTarget.put()));

        // Get a fence to wait for render work to complete
        winrt::handle fenceHandle;
        winrt::check_hresult(d3dFence->CreateSharedHandle(nullptr, GENERIC_ALL, nullptr, fenceHandle.put()));
        winrt::com_ptr<winrt::IInspectable> displayFenceIInspectable;
        displayFenceIInspectable.capture(interopDevice, &IDisplayDeviceInterop::OpenSharedHandle, fenceHandle.get());
        winrt::DisplayFence fence = displayFenceIInspectable.as<winrt::DisplayFence>();

        // Callback to any tools which need to perform operations post mode selection, post surface creation, but before
        // actual scan out starts.
        auto renderSetupArgs = winrt::make<RenderSetupToolArgs>(m_properties.as<IDisplayEngineProperties>());
        if (m_renderSetupCallback)
            m_renderSetupCallback(*this, renderSetupArgs);

        // Create the callback args object for per-frame rendering tools
        auto renderLoopArgs = winrt::make_self<RenderingToolArgs>(m_properties.as<IDisplayEngineProperties>());

        // Render and present until termination is signaled
        while (m_valid)
        {
            auto d3dContext4 = d3dContext.as<ID3D11DeviceContext4>();
            d3dContext4->Signal(d3dFence.get(), ++d3dFenceValue);

            // Callback to any tools which need to perform per-scanout operations
            if (m_renderLoopCallback)
                m_renderLoopCallback(*this, renderLoopArgs.as<IRenderingToolArgs>());

            winrt::DisplayTask task = taskPool.CreateTask();
            task.SetScanout(primaryScanout);
            task.SetWait(fence, d3dFenceValue);
            taskPool.ExecuteTask(task);
            m_displayDevice.WaitForVBlank(source);

            m_presenting = true;

            // Increment the frame counter
            renderLoopArgs->FrameNumber(++frameCounter);
        }

        Logger().LogNote(L"Stopping Render");
    }

    // DisplayEngineProperties
    DisplayEngineProperties::DisplayEngineProperties() :
        m_resolution({0, 0}), m_refreshRate(0.), m_mode(nullptr), m_requeryMode(true)
    {
    }

    winrt::DisplayModeInfo DisplayEngineProperties::ActiveMode()
    {
        return m_mode;
    }

    void DisplayEngineProperties::ActiveMode(winrt::DisplayModeInfo mode)
    {
        m_mode = std::move(mode);
        m_requeryMode = false;
    }

    double DisplayEngineProperties::RefreshRate()
    {
        return m_refreshRate;
    }

    void DisplayEngineProperties::RefreshRate(double rate)
    {
        m_refreshRate = rate;
        m_requeryMode = true;
    }

    winrt::Windows::Graphics::SizeInt32 DisplayEngineProperties::Resolution()
    {
        return m_resolution;
    }

    void DisplayEngineProperties::Resolution(winrt::Windows::Graphics::SizeInt32 resolution)
    {
        m_resolution = resolution;
        m_requeryMode = true;
    }

    winrt::com_array<winrt::IDisplayEnginePlaneProperties> DisplayEngineProperties::GetPlaneProperties()
    {
        std::vector<winrt::IDisplayEnginePlaneProperties> retVector;
        for (auto plane : m_planeProperties)
        {
            retVector.push_back(plane.as<winrt::IDisplayEnginePlaneProperties>());
        }
        return winrt::com_array<winrt::IDisplayEnginePlaneProperties>(retVector);
    }

    Windows::Foundation::Collections::IMap<hstring, IInspectable> DisplayEngineProperties::Properties()
    {
        return m_properties;
    }

    bool DisplayEngineProperties::RequeryMode()
    {
        return m_requeryMode;
    }

    DisplayEnginePlaneProperties::DisplayEnginePlaneProperties() :
        m_Properties(winrt::single_threaded_map<winrt::hstring, winrt::IInspectable>())
    {
    }

    // DisplayEnginePlaneProperties
    bool DisplayEnginePlaneProperties::Active()
    {
        return m_active;
    }

    void DisplayEnginePlaneProperties::Active(bool active)
    {
        m_active = active;
    }

    winrt::RectInt32 DisplayEnginePlaneProperties::Rect()
    {
        return m_rect;
    }

    void DisplayEnginePlaneProperties::Rect(winrt::RectInt32 bounds)
    {
        m_rect = bounds;
    }

    winrt::Windows::Foundation::Collections::IMap<winrt::hstring, winrt::IInspectable> DisplayEnginePlaneProperties::Properties()
    {
        return m_Properties;
    }

    HRESULT __stdcall DisplayEnginePlaneProperties::GetPlaneTexture(ID3D11Texture2D** texture) noexcept
    {
        m_planeTexture->AddRef();
        *texture = m_planeTexture.get();

        return S_OK;
    }

    void DisplayEnginePlaneProperties::SetPlaneTexture(ID3D11Texture2D* texture)
    {
        m_planeTexture.copy_from(texture);
    }

    void DisplaySetupToolArgs::IsModeCompatible(bool accept)
    {
        if (m_compatibility && !accept)
        {
            m_compatibility = false;
        }
    }

} // namespace winrt::BasicDisplayControl::implementation

#pragma warning(pop)