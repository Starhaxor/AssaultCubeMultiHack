#include "pch.h"

#include <cstdint>

#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_opengl2.h"
#include "imgui_impl_win32.h"

#include "menu.h"
#include "menu_state.h"
#include "render_hooks.h"
#include "hook.h"
#include "hacks.h"

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

	// The engine re-centers the mouse every frame (SetCursorPos / ClipCursor /
	// SDL_WarpMouse). Hook them so the cursor is actually free while the menu
	// is open. Installed once, active only when menu::show.
	using SetCursorPos_t = BOOL(WINAPI*)(int, int);
	using ClipCursor_t = BOOL(WINAPI*)(const RECT*);
	using SdlWarpMouse_t = void(__cdecl*)(uint16_t, uint16_t);

	SetCursorPos_t oSetCursorPos = nullptr;
	ClipCursor_t oClipCursor = nullptr;
	SdlWarpMouse_t oSdlWarpMouse = nullptr;

	void* g_setCursorPosTarget = nullptr;
	void* g_clipCursorTarget = nullptr;
	void* g_sdlWarpTarget = nullptr;
	bool s_cursorHooksDone = false;

	struct EnumCtx { HWND found; };

	BOOL CALLBACK EnumGameWindows(HWND hwnd, LPARAM lParam)
	{
		DWORD pid = 0;
		GetWindowThreadProcessId(hwnd, &pid);
		if (pid != GetCurrentProcessId())
			return TRUE;
		if (!IsWindowVisible(hwnd))
			return TRUE;
		if (GetWindow(hwnd, GW_OWNER) != nullptr)
			return TRUE; // skip owned popups/tooltips
		LONG_PTR style = GetWindowLongPtrA(hwnd, GWL_STYLE);
		if (!(style & WS_CAPTION))
			return TRUE; // need a real top-level window
		reinterpret_cast<EnumCtx*>(lParam)->found = hwnd;
		return FALSE;
	}

	HWND FindGameWindow()
	{
		if (g_gameHwnd && IsWindow(g_gameHwnd))
			return g_gameHwnd;

		// Cached window died (video mode change) -> drop stale hook state.
		g_gameHwnd = nullptr;
		oWndProc = nullptr;
		s_cursorUnlocked = false;

		HWND hwnd = FindWindowA(nullptr, "AssaultCube");
		if (!hwnd)
			hwnd = FindWindowA("SDL_app", nullptr);
		if (!hwnd)
		{
			// Title/class lookup failed (e.g. injector was foreground at init):
			// fall back to our own process's main window.
			EnumCtx ctx{};
			EnumWindows(&EnumGameWindows, reinterpret_cast<LPARAM>(&ctx));
			hwnd = ctx.found;
		}
		if (!hwnd)
			hwnd = GetForegroundWindow();
		if (hwnd)
			g_gameHwnd = hwnd;
		return hwnd;
	}

	// AssaultCube uses SDL 1.2: the engine grabs the mouse via
	// SDL_WM_GrabInput, which re-clips + hides the cursor and feeds
	// relative motion. Fighting it with ClipCursor alone loses, so ask
	// SDL itself to release/acquire the grab.
	using SdlGrabInput_t = int(__cdecl*)(int); // SDL_GrabMode: OFF=0, ON=1

	SdlGrabInput_t GetSdlGrabInput()
	{
		static bool tried = false;
		static SdlGrabInput_t fn = nullptr;
		if (!tried)
		{
			tried = true;
			HMODULE sdl = GetModuleHandleA("SDL.dll");
			if (!sdl)
				sdl = GetModuleHandleA("SDL12.dll");
			if (sdl)
				fn = reinterpret_cast<SdlGrabInput_t>(GetProcAddress(sdl, "SDL_WM_GrabInput"));
		}
		return fn;
	}

	bool IsMovementKey(WPARAM vk)
	{
		switch (vk)
		{
		case 'W': case 'A': case 'S': case 'D':
		case 'C':
		case VK_SPACE: case VK_SHIFT: case VK_CONTROL:
		case VK_UP: case VK_DOWN: case VK_LEFT: case VK_RIGHT:
			return true;
		default:
			return false;
		}
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
			// Let movement keys through so the player can walk/jump with
			// the menu open (aiming/shooting stay blocked below).
			if ((msg == WM_KEYDOWN || msg == WM_KEYUP) && IsMovementKey(wParam))
				return CallWindowProcA(oWndProc, hWnd, msg, wParam, lParam);
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
		if (SdlGrabInput_t grab = GetSdlGrabInput())
			grab(0); // SDL_GRAB_OFF: SDL itself unclips + shows the cursor
		ClipCursor(nullptr);
		int guard = 0;
		while (ShowCursor(TRUE) < 1 && ++guard < 20) {}
		SetCursor(LoadCursor(nullptr, IDC_ARROW));
		s_cursorUnlocked = true;
	}

	void RestoreCursorForGame()
	{
		if (SdlGrabInput_t grab = GetSdlGrabInput())
			grab(1); // SDL_GRAB_ON: give FPS mouse-look back
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

	BOOL WINAPI hkSetCursorPos(int X, int Y)
	{
		if (menu::show)
			return TRUE; // swallow the engine's per-frame recenter
		return oSetCursorPos ? oSetCursorPos(X, Y) : FALSE;
	}

	BOOL WINAPI hkClipCursor(const RECT* lpRect)
	{
		if (menu::show)
			return oClipCursor ? oClipCursor(nullptr) : FALSE; // force unclipped
		return oClipCursor ? oClipCursor(lpRect) : FALSE;
	}

	void __cdecl hkSdlWarpMouse(uint16_t x, uint16_t y)
	{
		if (menu::show)
			return; // swallow SDL recenter
		if (oSdlWarpMouse)
			oSdlWarpMouse(x, y);
	}

	// Hook the recenter APIs once. Each install is null-checked: if our
	// disassembler can't handle a prologue, that hook is skipped and the
	// remaining ones still protect the cursor.
	void HookCursorApis()
	{
		if (s_cursorHooksDone)
			return;
		s_cursorHooksDone = true;

		if (HMODULE user32 = GetModuleHandleA("user32.dll"))
		{
			if (void* t = GetProcAddress(user32, "SetCursorPos"))
			{
				g_setCursorPosTarget = t;
				oSetCursorPos = static_cast<SetCursorPos_t>(
					hook::trampoline(t, reinterpret_cast<void*>(&hkSetCursorPos)));
			}
			if (void* t = GetProcAddress(user32, "ClipCursor"))
			{
				g_clipCursorTarget = t;
				oClipCursor = static_cast<ClipCursor_t>(
					hook::trampoline(t, reinterpret_cast<void*>(&hkClipCursor)));
			}
		}
		HMODULE sdl = GetModuleHandleA("SDL.dll");
		if (!sdl)
			sdl = GetModuleHandleA("SDL12.dll");
		if (sdl)
		{
			if (void* t = GetProcAddress(sdl, "SDL_WarpMouse"))
			{
				g_sdlWarpTarget = t;
				oSdlWarpMouse = static_cast<SdlWarpMouse_t>(
					hook::trampoline(t, reinterpret_cast<void*>(&hkSdlWarpMouse)));
			}
		}
	}

	void UnhookCursorApis()
	{
		if (g_setCursorPosTarget) hook::uninstall(g_setCursorPosTarget);
		if (g_clipCursorTarget) hook::uninstall(g_clipCursorTarget);
		if (g_sdlWarpTarget) hook::uninstall(g_sdlWarpTarget);
		g_setCursorPosTarget = g_clipCursorTarget = g_sdlWarpTarget = nullptr;
		oSetCursorPos = nullptr;
		oClipCursor = nullptr;
		oSdlWarpMouse = nullptr;
		s_cursorHooksDone = false;
	}
}

HRESULT APIENTRY hkEndScene(IDirect3DDevice9* device)
{
	UpdateCursorForMenu();
	if (menu::Initialize(FindGameWindow(), device))
	{
		HookWndProc(FindGameWindow());
		HookCursorApis();
		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		menu::Render();
		hacks::RunAimbot();
		hacks::RenderOverlay();

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
		HookCursorApis();

		ImGui_ImplOpenGL2_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		menu::Render();
		hacks::RunAimbot();
		hacks::RenderOverlay();

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
	UnhookCursorApis();
	if (oWndProc && g_gameHwnd && IsWindow(g_gameHwnd))
		SetWindowLongPtrA(g_gameHwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(oWndProc));
	oWndProc = nullptr;
	menu::Shutdown();
}
