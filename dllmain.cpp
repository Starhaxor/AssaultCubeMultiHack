#include "pch.h"

#include <d3d9.h>

#include "kiero.hpp"
#include "kiero_d3d9.hpp"

#include "hook.h"
#include "render_hooks.h"

// Direct3D9 vtable indices for the device methods we hook.
constexpr int ENDSCENE_INDEX = 42; // IDirect3DDevice9::EndScene
constexpr int RESET_INDEX    = 16; // IDirect3DDevice9::Reset

static DWORD WINAPI MainThread(LPVOID)
{
    // AssaultCube renders with OpenGL (opengl32.dll), NOT D3D9.
    // Old code waited forever for d3d9.dll, so no hook was ever installed.
    for (int i = 0; i < 300; ++i)
    {
        if (GetModuleHandleA("opengl32.dll") || GetModuleHandleA("d3d9.dll"))
            break;
        Sleep(100);
    }

    bool hooked = false;

    // Primary path: hook the game's actual presenter.
    if (HMODULE gl = GetModuleHandleA("opengl32.dll"))
    {
        if (void* target = GetProcAddress(gl, "wglSwapBuffers"))
        {
            g_wglSwapBuffersTarget = target;
            oWglSwapBuffers = static_cast<WglSwapBuffers_t>(
                hook::trampoline(target, reinterpret_cast<void*>(&hkWglSwapBuffers)));
            hooked = hooked || (oWglSwapBuffers != nullptr);
        }
    }

    // Fallback: keep the D3D9 hook for D3D9 games (opportunistic, never blocks).
    if (GetModuleHandleA("d3d9.dll"))
    {
        kiero::D3D9Output output;
        if (kiero::locate<kiero::Implementation_D3D9>(nullptr, &output) == kiero::Error_Nil &&
            output.device_methods.size() > ENDSCENE_INDEX &&
            output.device_methods.size() > RESET_INDEX)
        {
            g_endSceneTarget = output.device_methods[ENDSCENE_INDEX];
            g_resetTarget    = output.device_methods[RESET_INDEX];

            oEndScene = static_cast<EndScene_t>(hook::trampoline(g_endSceneTarget, reinterpret_cast<void*>(&hkEndScene)));
            oReset = static_cast<Reset_t>(hook::trampoline(g_resetTarget, reinterpret_cast<void*>(&hkReset)));
            hooked = hooked || (oEndScene && oReset);
        }
    }

    return hooked ? 0 : 1;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0, MainThread, nullptr, 0, nullptr);
        break;
    case DLL_PROCESS_DETACH:
        render_uninstall();
        if (g_wglSwapBuffersTarget) hook::uninstall(g_wglSwapBuffersTarget);
        if (g_endSceneTarget) hook::uninstall(g_endSceneTarget);
        if (g_resetTarget)    hook::uninstall(g_resetTarget);
        break;
    }
    return TRUE;
}