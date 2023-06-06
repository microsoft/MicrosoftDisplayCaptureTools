#include "pch.h"

namespace winrt
{
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Windows::Foundation::Collections;
    using namespace winrt::Windows::Devices::Display::Core;
} // namespace winrt

namespace winrt::TanagerPlugin::DisplayHelpers 
{
IAsyncAction WaitForDisplayDevicesChange()
{
    auto displayManager = winrt::DisplayManager::Create(DisplayManagerOptions::None);

    winrt::handle changedEvent{CreateEventW(nullptr, TRUE, FALSE, nullptr)};

    auto changedToken = displayManager.Changed([&](const auto&, IDisplayManagerChangedEventArgs args) {
        args.Handled(true);
        SetEvent(changedEvent.get());
    });

    auto enabledToken = displayManager.Enabled([](const auto&, IDisplayManagerEnabledEventArgs args) { args.Handled(true); });
    auto disabledToken = displayManager.Disabled([](const auto&, IDisplayManagerDisabledEventArgs args) { args.Handled(true); });
    auto pathsToken = displayManager.PathsFailedOrInvalidated(
        [](const auto&, IDisplayManagerPathsFailedOrInvalidatedEventArgs args) { args.Handled(true); });

    displayManager.Start();

    // After the changed event has been registered, suspend until the changed event is fired
    co_await winrt::resume_on_signal(changedEvent.get());

    displayManager.Stop();

    co_return;
}
} // namespace winrt::TanagerPlugin::DisplayHelpers