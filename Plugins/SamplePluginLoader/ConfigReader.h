#pragma once

#include <memory>
#include "winrt/Windows.Data.Json.h"
#include <winrt/Windows.Graphics.h>

struct DisplayPath
{
	winrt::Windows::Graphics::DisplayAdapterId adapterId;
	uint32_t targetId;
};

class ConfigReader
{
public:
	ConfigReader(std::wstring configPath);

	std::wstring GetPluginPath() { return m_pluginPath; }
	std::map<uint32_t, DisplayPath> GetDisplayMappings();
private:
	std::wstring m_pluginPath;
};