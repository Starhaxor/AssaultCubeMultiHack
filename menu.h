#pragma once

#include <windows.h>
#include <d3d9.h>

namespace menu
{
	bool Initialize(HWND hwnd, IDirect3DDevice9* device);
	bool InitializeGL(HWND hwnd);
	void Shutdown();
	bool IsInitialized();
	void Render();
	void InvalidateDeviceObjects();
	void CreateDeviceObjects();
}
