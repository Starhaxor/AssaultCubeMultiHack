#include "pch.h"

#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_opengl2.h"
#include "imgui_impl_win32.h"

#include "menu.h"
#include "menu_state.h"
#include "render_hooks.h"

EndScene_t oEndScene = nullptr;
Reset_t oReset = nullptr;
WglSwapBuffers_t oWglSwapBuffers = nullptr;

void* g_endSceneTarget = nullptr;
void* g_resetTarget = nullptr;
void* g_wglSwapBuffersTarget = nullptr;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace
{
	HWND g_gameHwnd = nullptr;
	WNDPROC oWndProc = nullptr;
	bool s_cursorUnlocked = false;

	HWND FindGameWindow()
	{
		if (g_gameHwnd && IsWindow(g_gameHwnd))
			return g_gameHwnd;

		HWND hwnd = FindWindowA(nullptr, "AssaultCube");
		if (!hwnd)
			hwnd = FindWindowA("SDL_app", nullptr);
		if (hwnd)
			g_gameHwnd = hwnd;
		if (!hwnd)
			hwnd = GetForegroundWindow();
		return hwnd;
	}

	LRESULT CALLBACK hkWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		// Always feed ImGui first so the menu tracks mouse/keyboard.
		ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);

		if (menu::show)
		{
			// Keep a visible arrow cursor while the menu is open.
			if (msg == WM_SETCURSOR)
			{
				SetCursor(LoadCursor(nullptr, IDC_ARROW));
				return TRUE;
			}
			// Swallow game input so the camera doesn't rotate / weapons don't
			// fire while clicking the menu. System keys (Alt+Tab etc.) still
			// go to the game.
			switch (msg)
			{
			case WM_MOUSEMOVE:
			case WM_LBUTTONDOWN: case WM_LBUTTONUP: case WM_LBUTTONDBLCLK:
			case WM_RBUTTONDOWN: case WM_RBUTTONUP: case WM_RBUTTONDBLCLK:
			case WM_MBUTTONDOWN: case WM_MBUTTONUP: case WM_MBUTTONDBLCLK:
			case WM_XBUTTONDOWN: case WM_XBUTTONUP: case WM_XBUTTONDBLCLK:
			case WM_MOUSEWHEEL: case WM_MOUSEHWHEEL:
			case WM_KEYDOWN: case WM_KEYUP:
			case WM_CHAR: case WM_DEADCHAR: case WM_SYSCHAR:
			case WM_INPUT:
				return 0;
			default:
				break;
			}
		}
		return CallWindowProcA(oWndProc, hWnd, msg, wParam, lParam);
	}

	void HookWndProc(HWND hwnd)
	{
		if (oWndProc || !hwnd)
			return;
		oWndProc = reinterpret_cast<WNDPROC>(
			SetWindowLongPtrA(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&hkWndProc)));
		if (oWndProc == nullptr)
			oWndProc = nullptr;
		else
			g_gameHwnd = hwnd;
	}

	void UnlockCursorForMenu()
	{
		ClipCursor(nullptr);
		int guard = 0;
		while (ShowCursor(TRUE) < 1 && ++guard < 20) {}
		SetCursor(LoadCursor(nullptr, IDC_ARROW));
		s_cursorUnlocked = true;
	}

	void RestoreCursorForGame()
	{
		int guard = 0;
		while (ShowCursor(FALSE) >= 0 && ++guard < 20) {}
		s_cursorUnlocked = false;
	}

	// AssaultCube grabs + hides the cursor for FPS mouse-look. Free it every
	// frame while the menu is open, hand it back when the menu closes.
	void UpdateCursorForMenu()
	{
		if (menu::show)
		{
			ClipCursor(nullptr);
			if (!s_cursorUnlocked)
				UnlockCursorForMenu();
			SetCursor(LoadCursor(nullptr, IDC_ARROW));
		}
		else if (s_cursorUnlocked)
		{
			RestoreCursorForGame();
		}
	}
}

HRESULT APIENTRY hkEndScene(IDirect3DDevice9* device)
{
	UpdateCursorForMenu();
	if (menu::Initialize(FindGameWindow(), device))
	{
		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		menu::Render();

		ImGui::EndFrame();
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	}

	return oEndScene ? oEndScene(device) : D3DERR_INVALIDCALL;
}

HRESULT APIENTRY hkReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* params)
{
	menu::InvalidateDeviceObjects();

	HRESULT hr = oReset ? oReset(device, params) : D3DERR_INVALIDCALL;
	if (SUCCEEDED(hr))
		menu::CreateDeviceObjects();

	return hr;
}

// AssaultCube renders with OpenGL (opengl32.dll), not D3D9, so this is the
// hook that actually fires in-game. Runs with the game's GL context current.
BOOL WINAPI hkWglSwapBuffers(HDC hdc)
{
	UpdateCursorForMenu();
	if (menu::InitializeGL(FindGameWindow()))
	{
		HookWndProc(FindGameWindow());

		ImGui_ImplOpenGL2_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		menu::Render();

		ImGui::EndFrame();
		ImGui::Render();
		ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
	}

	return oWglSwapBuffers ? oWglSwapBuffers(hdc) : FALSE;
}

void render_uninstall()
{
	if (s_cursorUnlocked)
		RestoreCursorForGame();
	if (oWndProc && g_gameHwnd && IsWindow(g_gameHwnd))
		SetWindowLongPtrA(g_gameHwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(oWndProc));
	oWndProc = nullptr;
	menu::Shutdown();
}
