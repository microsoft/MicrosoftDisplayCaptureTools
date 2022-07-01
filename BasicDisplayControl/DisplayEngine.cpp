#include "pch.h"
#include "DisplayEngine.h"
#include "DisplayEngine.g.cpp"
#include "DisplayEngineFactory.g.cpp"

namespace winrt
{
    // Standard WinRT inclusions
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Windows::Foundation::Collections;

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
}

namespace MonitorUtilities
{
    static LUID LuidFromAdapterId(winrt::Windows::Graphics::DisplayAdapterId id)
    {
        return { id.LowPart, id.HighPart };
    }

    class MonitorControl
    {
    public:
        MonitorControl(LUID adapterId, UINT targetId) :
            m_luid(adapterId), m_targetId(targetId), m_removeSpecializationOnExit(true)
        {
            DISPLAYCONFIG_GET_MONITOR_SPECIALIZATION getSpecialization{};
            getSpecialization.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_MONITOR_SPECIALIZATION;
            getSpecialization.header.id = m_targetId;
            getSpecialization.header.adapterId = m_luid;
            getSpecialization.header.size = sizeof(getSpecialization);

            winrt::check_win32(DisplayConfigGetDeviceInfo(&getSpecialization.header));

            if (0 == getSpecialization.isSpecializationAvailableForSystem)
            {
                printf("Monitor specialization is not available on this system - have you enabled testsigning?\n");
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
    };
}

namespace winrt::DisplayControl::implementation
{
    winrt::IDisplayEngine DisplayEngineFactory::CreateDisplayEngine(winrt::ILogger const& logger)
    {
        return winrt::make<DisplayEngine>(logger);
    }

    DisplayEngine::DisplayEngine(winrt::ILogger const& logger) : 
        m_logger(logger)
    {
        m_displayManager = winrt::DisplayManager::Create(winrt::DisplayManagerOptions::None);

        m_logger.LogNote(L"DisplayEngine " + Name() + L" Instantiated");
    }

    DisplayEngine::DisplayEngine()
    {
        // Throw - callers should explicitly instantiate through the factory
        throw winrt::hresult_illegal_method_call();
    }

    DisplayEngine::~DisplayEngine()
    {

    }

    hstring DisplayEngine::Name()
    {
        return L"BasicDisplayControl";
    }

    void DisplayEngine::InitializeForDisplayTarget(winrt::DisplayTarget const& target)
    {
        // Reset the currently tracked target to the supplied one
        m_displayTarget = target;
        RefreshTarget();

        if (!target)
        {
            // Nothing else to do.
            return;
        }

        // Remove the targeted display from composition.
        m_monitorControl = std::make_unique<MonitorUtilities::MonitorControl>(
            MonitorUtilities::LuidFromAdapterId(m_displayTarget.Adapter().Id()),
            m_displayTarget.AdapterRelativeId());

        ConnectTarget();

        // Instantiate the Capabilities and Properties objects
        m_capabilities = winrt::make_self<DisplayEngineCapabilities>();
        m_propertySet = winrt::make_self<DisplayEnginePropertySet>();

        // Get the display capabilities
        PopulateCapabilities();

        // Ensure that there are the same number of planes on both the capabilities and property trees
        for (auto planes : m_capabilities->m_planeCapabilities)
        {
            auto planeProperties = winrt::make_self<DisplayEnginePlanePropertySet>();
            m_propertySet->m_planeProperties.push_back(planeProperties);
        }

        // Mark the base plane as active
        m_propertySet->m_planeProperties[0]->Active(true);
    }

    void DisplayEngine::InitializeForStableMonitorId(winrt::hstring target)
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
            // TODO - log this case
            throw winrt::hresult_invalid_argument();
        }

        InitializeForDisplayTarget(chosenTarget);
    }

    winrt::DisplayTarget DisplayEngine::GetTarget()
    {
        return m_displayTarget;
    }

    winrt::IDisplayEngineCapabilities DisplayEngine::GetCapabilities()
    {
        return *m_capabilities;
    }

    winrt::IDisplayEnginePropertySet DisplayEngine::GetProperties()
    {
        return *m_propertySet;
    }

    winrt::IDisplayEnginePrediction DisplayEngine::GetPrediction()
    {
        auto prediction = make_self<DisplayEnginePrediction>(m_propertySet.get());

        return *prediction;
    }

    void DisplayEngine::SetConfigData(IJsonValue data)
    {

    }

    winrt::IClosable DisplayEngine::StartRender()
    {
        // Re-connect the target
        ConnectTarget();

        auto renderer = make_self<Renderer>();

        renderer->displayDevice = m_displayDevice;

        renderer->displayManager = m_displayManager;
        renderer->displayTarget  = m_displayTarget;
        renderer->displayState   = m_displayState;
        renderer->displayPath    = m_displayPath;

        renderer->StartRender(m_propertySet.get());
        return renderer.as<IClosable>();
    }

    void DisplayEngine::RefreshTarget()
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
            // TODO: log this case
            throw winrt::hresult_changed_state();
        }
    }

    void DisplayEngine::ConnectTarget()
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
            // TODO: log this case and throw a more useful error
            throw winrt::hresult_error();
        }

        m_displayState = result.State();
        m_displayPath = m_displayState.ConnectTarget(m_displayTarget);
        m_displayDevice = m_displayManager.CreateDisplayDevice(m_displayTarget.Adapter());
    }

    void DisplayEngine::PopulateCapabilities()
    {
        // Create the capabilities objects for the base plane (the only plane supported by this implementation)
        auto basePlaneCapabilities = winrt::make_self<DisplayEnginePlaneCapabilities>();
        m_capabilities->m_planeCapabilities.push_back(basePlaneCapabilities);

        // Populate the mode list
        auto modeList = m_displayPath.FindModes(winrt::DisplayModeQueryOptions::None);
        m_capabilities->m_modes.assign(modeList.begin(), modeList.end());
    }

    // DisplayEngineCapabilities
    DisplayEngineCapabilities::DisplayEngineCapabilities()
    {
    }

    winrt::com_array<winrt::DisplayModeInfo> DisplayEngineCapabilities::GetSupportedModes()
    {
        return winrt::com_array<winrt::DisplayModeInfo>(m_modes);
    }

    winrt::com_array<winrt::IDisplayEnginePlaneCapabilities> DisplayEngineCapabilities::GetPlaneCapabilities()
    {
        std::vector<winrt::IDisplayEnginePlaneCapabilities> retVector;
        for (auto plane : m_planeCapabilities)
        {
            retVector.push_back(plane.as<winrt::IDisplayEnginePlaneCapabilities>());
        }
        return winrt::com_array<winrt::IDisplayEnginePlaneCapabilities>(retVector);
    }

    // DisplayEnginePropertySet
    DisplayEnginePropertySet::DisplayEnginePropertySet() :
        m_resolution({ 0,0 }),
        m_refreshRate(0.),
        m_mode(nullptr),
        m_requeryMode(true)
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
        return winrt::BitmapBounds();
    }

    void DisplayEnginePlanePropertySet::Rect(winrt::BitmapBounds bounds)
    {
    }

    winrt::DirectXPixelFormat DisplayEnginePlanePropertySet::Format()
    {
        return winrt::DirectXPixelFormat::R8G8B8A8UIntNormalized;
    }

    void DisplayEnginePlanePropertySet::Format(winrt::DirectXPixelFormat format) 
    {
    }

    winrt::SoftwareBitmap DisplayEnginePlanePropertySet::SourceBitmap()
    {
        return nullptr;
    }

    void DisplayEnginePlanePropertySet::SourceBitmap(winrt::SoftwareBitmap bitmap)
    {
    }

    PixelColor DisplayEnginePlanePropertySet::ClearColor()
    {
        return m_color;
    }

    void DisplayEnginePlanePropertySet::ClearColor(PixelColor clearColor)
    {
        m_color = clearColor;
    }

    // Renderer
    void Renderer::Close()
    {
        if (m_valid)
        {
            m_valid = false;

            if (renderThread.joinable())
            {
                renderThread.join();
            }
        }
    }

    void Renderer::StartRender(DisplayEnginePropertySet* properties)
    {
        m_presenting = false;

        m_properties = properties;

        RefreshMode();

        displayPath.ApplyPropertiesFromMode(m_properties->ActiveMode());
        auto result = displayState.TryApply(winrt::DisplayStateApplyOptions::None);

        renderThread = std::thread(&Renderer::Render, this);

        // don't return until the render thread has started presenting
        while (!m_presenting)
        {
            std::this_thread::yield();
        }
    }

    void Renderer::Render()
    {
        winrt::init_apartment();

        // Initialize D3D objects
        winrt::com_ptr<IDXGIFactory6> dxgiFactory;
        dxgiFactory.capture(&CreateDXGIFactory2, 0);

        winrt::com_ptr<IDXGIAdapter4> dxgiAdapter;
        dxgiAdapter.capture(dxgiFactory, &IDXGIFactory7::EnumAdapterByLuid, MonitorUtilities::LuidFromAdapterId(displayTarget.Adapter().Id()));

        D3D_FEATURE_LEVEL featureLevel;
        winrt::com_ptr<ID3D11Device> device;
        winrt::check_hresult(D3D11CreateDevice(dxgiAdapter.get(), D3D_DRIVER_TYPE_UNKNOWN, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, device.put(), &featureLevel, m_d3dContext.put()));
        m_d3dDevice = device.as<ID3D11Device5>();

        m_d3dFence.capture(m_d3dDevice, &ID3D11Device5::CreateFence, 0, D3D11_FENCE_FLAG_SHARED);

        // Initialize DDisplay sources and tasks
        auto taskPool = displayDevice.CreateTaskPool();
        auto source = displayDevice.CreateScanoutSource(displayTarget);

        winrt::SizeInt32 sourceResolution = displayPath.SourceResolution().Value();
        winrt::Direct3D11::Direct3DMultisampleDescription multisampleDesc = {};
        multisampleDesc.Count = 1;

        // Create a surface format description for the primaries
        winrt::DisplayPrimaryDescription primaryDesc{
            static_cast<uint32_t>(sourceResolution.Width), static_cast<uint32_t>(sourceResolution.Height),
            displayPath.SourcePixelFormat(), winrt::DirectXColorSpace::RgbFullG22NoneP709,
            false,
            multisampleDesc };

        winrt::DisplaySurface primarySurface{ nullptr };
        winrt::DisplayScanout primaryScanout{ nullptr };

        primarySurface = displayDevice.CreatePrimary(displayTarget, primaryDesc);
        primaryScanout = displayDevice.CreateSimpleScanout(source, primarySurface, 0, 1);

        auto interopDevice = displayDevice.as<IDisplayDeviceInterop>();
        auto rawSurface = primarySurface.as<::IInspectable>();

        winrt::handle primarySurfaceHandle;
        winrt::check_hresult(interopDevice->CreateSharedHandle(rawSurface.get(), nullptr, GENERIC_ALL, nullptr, primarySurfaceHandle.put()));

        m_d3dSurface.capture(m_d3dDevice, &ID3D11Device5::OpenSharedResource1, primarySurfaceHandle.get());

        D3D11_TEXTURE2D_DESC d3dTextureDesc{};
        m_d3dSurface->GetDesc(&d3dTextureDesc);

        D3D11_RENDER_TARGET_VIEW_DESC d3dRenderTargetViewDesc{};
        d3dRenderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        d3dRenderTargetViewDesc.Texture2D.MipSlice = 0;
        d3dRenderTargetViewDesc.Format = d3dTextureDesc.Format;

        // Create the render target view
        winrt::check_hresult(m_d3dDevice->CreateRenderTargetView(m_d3dSurface.get(), &d3dRenderTargetViewDesc, m_d3dRenderTarget.put()));

        // Get a fence to wait for render work to complete
        winrt::handle fenceHandle;
        winrt::check_hresult(m_d3dFence->CreateSharedHandle(nullptr, GENERIC_ALL, nullptr, fenceHandle.put()));
        winrt::com_ptr<::IInspectable> displayFenceIInspectable;
        displayFenceIInspectable.capture(interopDevice, &IDisplayDeviceInterop::OpenSharedHandle, fenceHandle.get());
        winrt::DisplayFence fence = displayFenceIInspectable.as<winrt::DisplayFence>();

        // Get the rendering properties (in this sample just the base plane clear color)
        auto basePlaneClearColor = m_properties->m_planeProperties[0]->ClearColor();

        // Render and present until termination is signalled
        while (m_valid)
        {
            {
                float clearColor[4] = { basePlaneClearColor.ChannelA, basePlaneClearColor.ChannelB, basePlaneClearColor.ChannelC, 1 };
                m_d3dContext->ClearRenderTargetView(m_d3dRenderTarget.get(), clearColor);
            }

            auto d3dContext4 = m_d3dContext.as<ID3D11DeviceContext4>();
            d3dContext4->Signal(m_d3dFence.get(), ++m_d3dFenceValue);

            winrt::DisplayTask task = taskPool.CreateTask();
            task.SetScanout(primaryScanout);
            task.SetWait(fence, m_d3dFenceValue);
            taskPool.ExecuteTask(task);
            displayDevice.WaitForVBlank(source);

            m_presenting = true;
        }
    }

    void Renderer::RefreshMode()
    {
        if (m_properties->RequeryMode())
        {
            auto modeList = displayPath.FindModes(winrt::DisplayModeQueryOptions::None);

            for (auto&& mode : modeList)
            {
                double presentationRate = static_cast<double>(mode.PresentationRate().VerticalSyncRate.Numerator) / 
                    static_cast<double>(mode.PresentationRate().VerticalSyncRate.Denominator);

                /*
                    TODO 38041452: remove this, it's only here because this has been handy in debugging bringup a few times and I didn't want
                    to write it out over and over again.

                    printf("\n\tmode\n\tResolution: (%d, %d)\n\tRefresh Rate: %f\n\tFormat: %s\n\n",
                    mode.TargetResolution().Width,
                    mode.TargetResolution().Height,
                    presentationRate,
                    mode.SourcePixelFormat() == DirectXPixelFormat::R8G8B8A8UIntNormalized ? "RGB8" : "not-RGB8");
                */

                double delta = fabs(presentationRate - m_properties->RefreshRate());
                
                if (mode.SourcePixelFormat() == DirectXPixelFormat::R8G8B8A8UIntNormalized && mode.IsInterlaced() == false &&
                    mode.TargetResolution().Height == m_properties->Resolution().Height &&
                    mode.TargetResolution().Width == m_properties->Resolution().Width)
                {

                    if (delta < sc_refreshRateEpsilon)
                    {
                        // we have a mode matching the requirements.
                        m_properties->ActiveMode(mode);
                        return;
                    }
                }
            }
            
            // No mode fit the set tools
            // TODO: log this case
            throw winrt::hresult_invalid_argument();
        }
    }

    //
    // Clears a pixel buffer to a set color.
    // 
    // Note: right now this operates in byte-sized increments, it can't currently handle pixel sizes not byte-aligned.
    //
    static void ClearPixelBuffer(IMemoryBufferReference buffer, PixelColor clearColor, DirectXPixelFormat format, uint32_t threadCount = 4)
    {
        // Ensure that the format is known and determine the slice size

        uint32_t pixelSize = 0;
        switch (format)
        {
        case DirectXPixelFormat::R8G8B8A8UIntNormalized:
            pixelSize = 4;
            break;
        default:
            throw winrt::hresult_not_implemented();
        }

        std::vector<std::thread> threads;
        uint32_t startingIndex = 0;
        uint32_t threadSectionSize = buffer.Capacity() / threadCount;

        for (uint32_t i = 0; i < threadCount; i++)
        {
            startingIndex = i * threadSectionSize;
            threads.push_back(std::thread([&](uint32_t index, uint32_t sectionSize)
                {
                    switch (format)
                    {
                    case DirectXPixelFormat::R8G8B8A8UIntNormalized:
                    {
                        struct PixelStruct
                        {
                            uint8_t r, g, b, a;
                        };

                        PixelStruct* pixelStruct = reinterpret_cast<PixelStruct*>(buffer.data()+index);

                        for (UINT i = index; i < (index + sectionSize) && i < buffer.Capacity(); i += sizeof(PixelStruct))
                        {
                            pixelStruct->r = static_cast<byte>(floorf(clearColor.ChannelA * 255.f + 0.5f));
                            pixelStruct->g = static_cast<byte>(floorf(clearColor.ChannelB * 255.f + 0.5f));
                            pixelStruct->b = static_cast<byte>(floorf(clearColor.ChannelC * 255.f + 0.5f));
                            pixelStruct->a = 255;

                            pixelStruct++;
                        }
                    }
                    break;
                    }
                }, startingIndex, threadSectionSize));
        }

        for (auto&& thread : threads)
        {
            thread.join();
        }
    }

    DisplayEnginePrediction::DisplayEnginePrediction(DisplayEnginePropertySet* properties)
    {
        auto mode = properties->ActiveMode();

        BitmapPixelFormat bitmapFormat;
        switch (mode.SourcePixelFormat())
        {
        case DirectXPixelFormat::R8G8B8A8UIntNormalized:
            bitmapFormat = BitmapPixelFormat::Rgba8;
            break;
        default:
            // Currently unhandled case - throw and log
            throw winrt::hresult_not_implemented();
        }

        m_bitmap = SoftwareBitmap(bitmapFormat, mode.TargetResolution().Width, mode.TargetResolution().Height, BitmapAlphaMode::Ignore);
        auto buffer = m_bitmap.LockBuffer(BitmapBufferAccessMode::Write);
        auto bufferReference = buffer.CreateReference();

        ClearPixelBuffer(bufferReference, properties->GetPlaneProperties()[0].ClearColor(), mode.SourcePixelFormat());
    }

    SoftwareBitmap DisplayEnginePrediction::GetBitmap()
    {
        return m_bitmap;
    }
}
