#include "pch.h"

#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_opengl2.h"
#include "imgui_impl_win32.h"

#include "menu.h"
#include "menu_state.h"
#include "menu_sections.h"
#include "theme.h"

namespace
{
	bool g_initialized = false;
	bool g_initializedGL = false;
}

namespace menu
{
	bool IsInitialized() { return g_initialized || g_initializedGL; }
	bool Initialize(HWND hwnd, IDirect3DDevice9* device)
	{
		if (g_initialized || !hwnd || !device)
			return g_initialized;

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::GetIO().IniFilename = nullptr; // no imgui.ini: sections always open fresh
		theme::ApplyHeritageCream();

		if (!ImGui_ImplWin32_Init(hwnd) || !ImGui_ImplDX9_Init(device))
		{
			ImGui_ImplDX9_Shutdown();
			ImGui_ImplWin32_Shutdown();
			ImGui::DestroyContext();
			return false;
		}

		g_initialized = true;
		return true;
	}

	bool InitializeGL(HWND hwnd)
	{
		if (g_initializedGL || g_initialized || !hwnd)
			return g_initializedGL;

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::GetIO().IniFilename = nullptr; // no imgui.ini: sections always open fresh
		theme::ApplyHeritageCream();

		if (!ImGui_ImplWin32_Init(hwnd) || !ImGui_ImplOpenGL2_Init())
		{
			ImGui_ImplOpenGL2_Shutdown();
			ImGui_ImplWin32_Shutdown();
			ImGui::DestroyContext();
			return false;
		}

		g_initializedGL = true;
		return true;
	}

	void Shutdown()
	{
		if (g_initialized)
		{
			ImGui_ImplDX9_Shutdown();
			ImGui_ImplWin32_Shutdown();
			ImGui::DestroyContext();
			g_initialized = false;
		}
		if (g_initializedGL)
		{
			ImGui_ImplOpenGL2_Shutdown();
			ImGui_ImplWin32_Shutdown();
			ImGui::DestroyContext();
			g_initializedGL = false;
		}
	}

	void Render()
	{
		if (!g_initialized && !g_initializedGL)
			return;

		if (GetAsyncKeyState(VK_INSERT) & 1)
			show = !show;

		if (!show)
			return;

		ImGui::SetNextWindowSize(ImVec2(360, 200), ImGuiCond_Once);
		ImGui::Begin("Assault Cube MultiHack", &show,
			ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);
		RenderPlayerSection();
		RenderAimbotSection();
		RenderVisualsSection();
		RenderMovementSection();
		RenderMiscSection();
		ImGui::End();
	}

	void InvalidateDeviceObjects()
	{
		if (g_initialized && !g_initializedGL)
			ImGui_ImplDX9_InvalidateDeviceObjects();
	}

	void CreateDeviceObjects()
	{
		if (g_initialized && !g_initializedGL)
			ImGui_ImplDX9_CreateDeviceObjects();
	}
}
