#pragma once

#include <d3d9.h>

using EndScene_t = HRESULT(APIENTRY*)(IDirect3DDevice9*);
using Reset_t = HRESULT(APIENTRY*)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);
using WglSwapBuffers_t = BOOL(WINAPI*)(HDC);

extern EndScene_t oEndScene;
extern Reset_t oReset;
extern WglSwapBuffers_t oWglSwapBuffers;

extern void* g_endSceneTarget;
extern void* g_resetTarget;
extern void* g_wglSwapBuffersTarget;

HRESULT APIENTRY hkEndScene(IDirect3DDevice9* device);
HRESULT APIENTRY hkReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* params);
BOOL WINAPI hkWglSwapBuffers(HDC hdc);

// Restores the Win32 WndProc hook and shuts down ImGui. Call on detach.
void render_uninstall();
