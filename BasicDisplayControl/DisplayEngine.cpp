#include "pch.h"
#include "DisplayEngine.h"
#include "DisplayEngine.g.cpp"
#include "DisplayEngineFactory.g.cpp"

namespace winrt
{
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

    namespace MonitorUtilities
{
    static LUID LuidFromAdapterId(winrt::Windows::Graphics::DisplayAdapterId id)
    {
        return { id.LowPart, id.HighPart };
    }

    class MonitorControl
    {
    public:
        MonitorControl(LUID adapterId, UINT targetId, winrt::ILogger const& logger) :
            m_luid(adapterId), m_targetId(targetId), m_removeSpecializationOnExit(true), m_logger(logger)
        {
            DISPLAYCONFIG_GET_MONITOR_SPECIALIZATION getSpecialization{};
            getSpecialization.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_MONITOR_SPECIALIZATION;
            getSpecialization.header.id = m_targetId;
            getSpecialization.header.adapterId = m_luid;
            getSpecialization.header.size = sizeof(getSpecialization);

            winrt::check_win32(DisplayConfigGetDeviceInfo(&getSpecialization.header));

            if (0 == getSpecialization.isSpecializationAvailableForSystem)
            {
                m_logger.LogError(L"Monitor specialization is not available - have you enabled testsigning?");
                throw winrt::hresult_error();
            }

            if (1 == getSpecialization.isSpecializationEnabled)
            {
                m_removeSpecializationOnExit = false;
            }
            else
            {
                Toggle();
            }
        }

        ~MonitorControl()
        {
            if (m_removeSpecializationOnExit)
            {
                Toggle(true);
            }
        }

    private:
        void Toggle(bool reset = false)
        {
            DISPLAYCONFIG_SET_MONITOR_SPECIALIZATION setSpecialization{};
            setSpecialization.header.type = DISPLAYCONFIG_DEVICE_INFO_SET_MONITOR_SPECIALIZATION;
            setSpecialization.header.id = m_targetId;
            setSpecialization.header.adapterId = m_luid;
            setSpecialization.header.size = sizeof(setSpecialization);

            setSpecialization.isSpecializationEnabled = reset ? 0 : 1;
            wsprintf(setSpecialization.specializationApplicationName, L"%s", L"HardwareHLK - BasicDisplayControl");
            setSpecialization.specializationType = GUID_MONITOR_OVERRIDE_PSEUDO_SPECIALIZED;
            setSpecialization.specializationSubType = GUID_NULL;

            winrt::check_win32(DisplayConfigSetDeviceInfo(&setSpecialization.header));
        }

        LUID m_luid;
        UINT m_targetId;
        bool m_removeSpecializationOnExit;
        const winrt::ILogger m_logger;
    };
}

namespace winrt::BasicDisplayControl::implementation
{
    winrt::IDisplayEngine DisplayEngineFactory::CreateDisplayEngine(winrt::ILogger const& logger)
    {
        return winrt::make<DisplayEngine>(logger);
    }

    DisplayEngine::DisplayEngine(winrt::ILogger const& logger) : 
        m_logger(logger)
    {
        m_displayManager = winrt::DisplayManager::Create(winrt::DisplayManagerOptions::None);
    }

    DisplayEngine::DisplayEngine()
    {
        // Throw - callers should explicitly instantiate through the factory
        throw winrt::hresult_illegal_method_call();
    }

    DisplayEngine::~DisplayEngine()
    {
    }

    void DisplayEngine::SetConfigData(IJsonValue data)
    {
    }

    IDisplayOutput DisplayEngine::InitializeOutput(winrt::DisplayTarget const& target)
    {
        auto output = winrt::make<DisplayOutput>(m_logger, target, m_displayManager);

        return output;
    }

    IDisplayOutput DisplayEngine::InitializeOutput(winrt::hstring target)
    {
        // Translate the target string to a DisplayTarget and call 'InitializeForDisplayTarget'
        winrt::DisplayTarget chosenTarget{ nullptr };
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
            m_logger.LogError(L"The chosen target was not found - was the configuration used generated for this machine?");
            throw winrt::hresult_invalid_argument();
        }

        return InitializeOutput(chosenTarget);
    }

    // DisplayOutput
    DisplayOutput::DisplayOutput(ILogger const& logger, DisplayTarget const& target, DisplayManager const& manager) :
        m_logger(logger), m_displayTarget(target), m_displayManager(manager)
    {
        // Refresh the display target
        RefreshTarget();

        if (!target)
        {
            // The chosen target was not found - was the config file generated for this machine?
            m_logger.LogError(L"Attempted to initialize with a null display target.");
            throw winrt::hresult_invalid_argument();
        }

        // If needed, remove the targeted display from composition.
        if (m_displayTarget.UsageKind() != winrt::DisplayMonitorUsageKind::SpecialPurpose)
        {
            m_monitorControl = std::make_unique<MonitorUtilities::MonitorControl>(
                MonitorUtilities::LuidFromAdapterId(m_displayTarget.Adapter().Id()), m_displayTarget.AdapterRelativeId(), m_logger);

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
                m_logger.LogError(L"Unable to locate the target display after marking it as 'Specialized'");
                throw winrt::hresult_changed_state();
            }

            m_displayTarget = refreshedTarget;
        }

        ConnectTarget();

        // Instantiate properties object
        m_propertySet = winrt::make_self<DisplayEnginePropertySet>(m_logger);
        
        // This DisplayEngine implementation only supports 1 plane
        auto planeProperties = winrt::make_self<DisplayEnginePlanePropertySet>(m_logger);
        m_propertySet->m_planeProperties.push_back(planeProperties);

        // Mark the base plane as active
        m_propertySet->m_planeProperties[0]->Active(true);
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
    winrt::IDisplayEnginePropertySet DisplayOutput::GetProperties()
    {
        return *m_propertySet;
    }

    winrt::IDisplayEnginePrediction DisplayOutput::GetPrediction()
    {
        auto prediction = make_self<DisplayEnginePrediction>(m_propertySet.get(), m_logger);

        return *prediction;
    }

    winrt::IClosable DisplayOutput::StartRender()
    {
        // Re-connect the target
        ConnectTarget();

        // Create an object to stop the rendering when destroyed/closed.
        auto renderWatchdog = make_self<RenderWatchdog>(this);

        // Start the rendering process - first this will determine elgible modes and then it will spin off a thread to run 
        // a render loop. This function won't return until the thread has started.
        PrepareRender();

        return renderWatchdog.as<IClosable>();
    }

    event_token DisplayOutput::DisplaySetupCallback(Windows::Foundation::EventHandler<IDisplayEnginePropertySet> const& handler)
    {
        return m_displaySetupCallback.add(handler);
    }

    void DisplayOutput::DisplaySetupCallback(event_token const& token) noexcept
    {
        m_displaySetupCallback.remove(token);
    }

    event_token DisplayOutput::RenderSetupCallback(Windows::Foundation::EventHandler<IDisplayEnginePropertySet> const& handler)
    {
        return m_renderSetupCallback.add(handler);
    }

    void DisplayOutput::RenderSetupCallback(event_token const& token) noexcept
    {
        m_renderSetupCallback.remove(token);
    }

    event_token DisplayOutput::RenderLoopCallback(Windows::Foundation::EventHandler<IDisplayEnginePropertySet> const& handler)
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
            m_logger.LogError(L"The selected target can no longer be found. Selected Target: " + m_displayTarget.StableMonitorId());
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
            m_logger.LogError(L"Failed to acquire control of the target. Error: " + result.ExtendedErrorCode());
            throw winrt::hresult_error();
        }

        m_displayState = result.State();
        m_displayPath = m_displayState.ConnectTarget(m_displayTarget);
        m_displayDevice = m_displayManager.CreateDisplayDevice(m_displayTarget.Adapter());
    }

    // Rendering
    void DisplayOutput::StopRender()
    {
        m_logger.LogNote(L"Stopping Renderer Thread");

        if (m_valid)
        {
            m_valid = false;

            if (m_renderThread.joinable())
            {
                m_renderThread.join();
            }
        }
    }

    void DisplayOutput::RefreshMode()
    {
        if (m_propertySet->RequeryMode())
        {
            auto modeList = m_displayPath.FindModes(winrt::DisplayModeQueryOptions::None);

            for (auto&& mode : modeList)
            {
                double presentationRate = static_cast<double>(mode.PresentationRate().VerticalSyncRate.Numerator) /
                                          static_cast<double>(mode.PresentationRate().VerticalSyncRate.Denominator);

                double delta = fabs(presentationRate - m_propertySet->RefreshRate());

                if (mode.SourcePixelFormat() == DirectXPixelFormat::R8G8B8A8UIntNormalized && mode.IsInterlaced() == false &&
                    mode.TargetResolution().Height == m_propertySet->Resolution().Height &&
                    mode.TargetResolution().Width == m_propertySet->Resolution().Width &&
                    mode.SourceResolution().Height == m_propertySet->Resolution().Height &&
                    mode.SourceResolution().Width == m_propertySet->Resolution().Width)
                {

                    if (delta < sc_refreshRateEpsilon)
                    {
                        std::wstringstream buf{};
                        buf << L"Mode chosen: source(" << mode.TargetResolution().Width << L" x " << mode.TargetResolution().Height
                            << L") target(" << mode.SourceResolution().Width << L" x " << mode.SourceResolution().Height << L")";

                        // we have a mode matching the requirements.
                        m_logger.LogNote(buf.str());

                        m_propertySet->ActiveMode(mode);
                        return;
                    }
                }
            }

            // No mode fit the set tools - this _may_ indicate an error, but it may also just indicate that we are attempting
            // to auto-configure. So log a warning instead of an error to assist the user.
            m_logger.LogWarning(L"No display mode fit the selected options");
            throw winrt::hresult_invalid_argument();
        }
    }

    void DisplayOutput::PrepareRender()
    {
        m_logger.LogNote(L"Preparing Renderer Thread");

        m_presenting = false;

        // Call back to any tools which need to apply settings before the mode is chosen and applied
        if (m_displaySetupCallback) m_displaySetupCallback(*this, m_propertySet.as<IDisplayEnginePropertySet>());

        RefreshMode();

        m_displayPath.ApplyPropertiesFromMode(m_propertySet->ActiveMode());
        auto result = m_displayState.TryApply(winrt::DisplayStateApplyOptions::None);

        m_renderThread = std::thread(&DisplayOutput::RenderLoop, this);

        // don't return until the render thread has started presenting
        while (!m_presenting)
        {
            std::this_thread::yield();
        }
    }

    void DisplayOutput::RenderLoop()
    {
        winrt::init_apartment();

        m_logger.LogNote(L"Starting Render");

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
        winrt::com_ptr<ID3D11Texture2D> d3dSurface;

        d3dSurface.capture(d3dDevice, &ID3D11Device5::OpenSharedResource1, primarySurfaceHandle.get());

        D3D11_TEXTURE2D_DESC d3dTextureDesc{};
        d3dSurface->GetDesc(&d3dTextureDesc);

        D3D11_RENDER_TARGET_VIEW_DESC d3dRenderTargetViewDesc{};
        d3dRenderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        d3dRenderTargetViewDesc.Texture2D.MipSlice = 0;
        d3dRenderTargetViewDesc.Format = d3dTextureDesc.Format;

        // Create the render target view
        winrt::check_hresult(d3dDevice->CreateRenderTargetView(d3dSurface.get(), &d3dRenderTargetViewDesc, d3dRenderTarget.put()));

        // Get a fence to wait for render work to complete
        winrt::handle fenceHandle;
        winrt::check_hresult(d3dFence->CreateSharedHandle(nullptr, GENERIC_ALL, nullptr, fenceHandle.put()));
        winrt::com_ptr<::IInspectable> displayFenceIInspectable;
        displayFenceIInspectable.capture(interopDevice, &IDisplayDeviceInterop::OpenSharedHandle, fenceHandle.get());
        winrt::DisplayFence fence = displayFenceIInspectable.as<winrt::DisplayFence>();

        /*
        // Dump the base plane pixels into a buffer on the target
        auto dxgiSurface = d3dSurface.as<IDXGISurface>();
        winrt::com_ptr<ID2D1Factory> d2dFactory;
        winrt::check_hresult(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, d2dFactory.put()));
        winrt::com_ptr<ID2D1RenderTarget> d2dTarget;
        D2D1_RENDER_TARGET_PROPERTIES d2dRtProperties;
        d2dRtProperties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;
        d2dRtProperties.pixelFormat.format = DXGI_FORMAT_UNKNOWN;
        d2dRtProperties.type = D2D1_RENDER_TARGET_TYPE_DEFAULT;
        d2dRtProperties.minLevel = D2D1_FEATURE_LEVEL_10;
        d2dRtProperties.dpiX = 96.f;
        d2dRtProperties.dpiY = 96.f;
        d2dRtProperties.usage = D2D1_RENDER_TARGET_USAGE_NONE;

        winrt::check_hresult(d2dFactory->CreateDxgiSurfaceRenderTarget(dxgiSurface.get(), d2dRtProperties, d2dTarget.put()));

        winrt::com_ptr<ID2D1Bitmap> d2dBitmap;
        D2D1_BITMAP_PROPERTIES d2dBitmapProperties;
        d2dBitmapProperties.pixelFormat.format = (DXGI_FORMAT)m_propertySet->m_planeProperties[0]->BaseImage().Format();
        d2dBitmapProperties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;
        d2dBitmapProperties.dpiX = 96.f;
        d2dBitmapProperties.dpiY = 96.f;

        D2D1_SIZE_U d2dBitmapSize;
        d2dBitmapSize.height = m_propertySet->m_planeProperties[0]->BaseImage().Resolution().Height;
        d2dBitmapSize.width = m_propertySet->m_planeProperties[0]->BaseImage().Resolution().Width;
        auto d2dBitmapRect = D2D1::RectF(0, 0, (FLOAT)d2dBitmapSize.width, (FLOAT)d2dBitmapSize.height);

        {
            auto bitmapBuffer = m_propertySet->m_planeProperties[0]->BaseImage().Pixels();

            winrt::check_hresult(d2dTarget->CreateBitmap(
                d2dBitmapSize, bitmapBuffer.data(), d2dBitmapSize.width * 4, d2dBitmapProperties, d2dBitmap.put()));
        }

        d2dTarget->BeginDraw();
        d2dTarget->DrawBitmap(d2dBitmap.get(), d2dBitmapRect, 1.f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, d2dBitmapRect);
        winrt::check_hresult(d2dTarget->EndDraw());
        */

        // Callback to any tools which need to perform operations post mode selection, post surface creation, but before
        // actual scan out starts.
        m_propertySet->GetPlaneProperties()[0].PropertyBag().Insert(hstring(L"D3DSurface"), winrt::make<Wrapper>(d3dSurface.get()));
        if (m_renderSetupCallback) m_renderSetupCallback(*this, m_propertySet.as<IDisplayEnginePropertySet>());

        // Render and present until termination is signalled
        while (m_valid)
        {
            auto d3dContext4 = d3dContext.as<ID3D11DeviceContext4>();
            d3dContext4->Signal(d3dFence.get(), ++d3dFenceValue);

            // Callback to any tools which need to perform per-scanout operations
            if (m_renderLoopCallback) m_renderLoopCallback(*this, m_propertySet.as<IDisplayEnginePropertySet>());

            winrt::DisplayTask task = taskPool.CreateTask();
            task.SetScanout(primaryScanout);
            task.SetWait(fence, d3dFenceValue);
            taskPool.ExecuteTask(task);
            m_displayDevice.WaitForVBlank(source);

            m_presenting = true;
        }

        m_logger.LogNote(L"Stopping Render");
    }

    // DisplayEnginePropertySet
    DisplayEnginePropertySet::DisplayEnginePropertySet(winrt::ILogger const& logger) :
        m_resolution({ 0,0 }),
        m_refreshRate(0.),
        m_mode(nullptr),
        m_requeryMode(true), 
        m_logger(logger)
    {
    }

    winrt::DisplayModeInfo DisplayEnginePropertySet::ActiveMode()
    {
        return m_mode;
    }

    void DisplayEnginePropertySet::ActiveMode(winrt::DisplayModeInfo mode)
    {
        m_mode = std::move(mode);
        m_requeryMode = false;
    }

    double DisplayEnginePropertySet::RefreshRate()
    {
        return m_refreshRate;
    }

    void DisplayEnginePropertySet::RefreshRate(double rate)
    {
        m_refreshRate = rate;
        m_requeryMode = true;
    }

    winrt::Windows::Graphics::SizeInt32 DisplayEnginePropertySet::Resolution()
    {
        return m_resolution;
    }

    void DisplayEnginePropertySet::Resolution(winrt::Windows::Graphics::SizeInt32 resolution)
    {
        m_resolution = resolution;
        m_requeryMode = true;
    }

    winrt::com_array<winrt::IDisplayEnginePlanePropertySet> DisplayEnginePropertySet::GetPlaneProperties()
    {
        std::vector<winrt::IDisplayEnginePlanePropertySet> retVector;
        for (auto plane : m_planeProperties)
        {
            retVector.push_back(plane.as<winrt::IDisplayEnginePlanePropertySet>());
        }
        return winrt::com_array<winrt::IDisplayEnginePlanePropertySet>(retVector);
    }

    bool DisplayEnginePropertySet::RequeryMode()
    {
        return m_requeryMode;
    }

    DisplayEnginePlanePropertySet::DisplayEnginePlanePropertySet(MicrosoftDisplayCaptureTools::Framework::ILogger const& logger) : 
        m_logger(logger),
        m_propertyBag(winrt::single_threaded_map<winrt::hstring, winrt::IInspectable>())
    {
    }

    // DisplayEnginePlanePropertySet
    bool DisplayEnginePlanePropertySet::Active()
    {
        return m_active;
    }

    void DisplayEnginePlanePropertySet::Active(bool active)
    {
        m_active = active;
    }

    winrt::BitmapBounds DisplayEnginePlanePropertySet::Rect()
    {
        return m_rect;
    }

    void DisplayEnginePlanePropertySet::Rect(winrt::BitmapBounds bounds)
    {
        m_rect = bounds;
    }

    winrt::DirectXPixelFormat DisplayEnginePlanePropertySet::Format()
    {
        return winrt::DirectXPixelFormat::R8G8B8A8UIntNormalized;
    }

    void DisplayEnginePlanePropertySet::Format(winrt::DirectXPixelFormat format) 
    {
    }

    winrt::Windows::Foundation::Collections::IMap<winrt::hstring, winrt::IInspectable> DisplayEnginePlanePropertySet::PropertyBag()
    {
        return m_propertyBag;
    }

    DisplayEnginePrediction::DisplayEnginePrediction(DisplayEnginePropertySet* properties, winrt::ILogger const& logger) : 
        m_logger(logger)
    {
        m_logger.LogNote(L"Creating Prediction object from properties.");
        auto mode = properties->ActiveMode();

        FrameDataDescription description;

        switch (mode.SourcePixelFormat())
        {
        case DirectXPixelFormat::R8G8B8A8UIntNormalized:
            description.BitsPerPixel = 32;
            break;
        default:
            // Currently unhandled case - throw and log
            throw winrt::hresult_not_implemented();
        }

        // TODO: tool needs to draw into this....
        /*
        description.Stride = properties->GetPlaneProperties()[0].BaseImage().Resolution().Width * description.BitsPerPixel;
        description.PixelFormat = mode.SourcePixelFormat();

        m_frameData = FrameData(m_logger);
        m_frameData.FormatDescription(description);
        m_frameData.Resolution(mode.TargetResolution());

        auto buffer = Buffer(properties->GetPlaneProperties()[0].BaseImage().Pixels().Length());
        
        // copy the memory over to the new buffer
        memcpy(
            buffer.data(),
            properties->GetPlaneProperties()[0].BaseImage().Pixels().data(),
            properties->GetPlaneProperties()[0].BaseImage().Pixels().Length());

        m_frameData.Data(buffer);
        */
    }

    IFrameData DisplayEnginePrediction::GetFrameData()
    {
        m_logger.LogNote(L"Fetching predicted frame data.");

        return m_frameData;
    }

} // namespace winrt::BasicDisplayControl::implementation

#pragma warning(pop)