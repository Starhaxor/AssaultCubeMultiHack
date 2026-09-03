#pragma once

#include <windows.h>
#include <d3d9.h>

// PENCERE ISKELETI (menu.cpp): ImGui baslatma + pencere acma/kapama.
// Icindeki dugmeler menu_sections.cpp'dedir, buraya dokunmana gerek yok.

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
