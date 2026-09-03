#pragma once
#include <windows.h>
#include <cstdint>

// HOOK MOTORU - dokunma. Fonksiyonlari arada yakalamak icin kullanilir
// (orn: wglSwapBuffers). Nasil kullanildigini gormek icin dllmain.cpp'ye bak.

namespace hook
{
    // Installs an inline (detour) hook for x86 (32-bit) targets.
    // 'target'   : address of the function to hook (e.g. IDirect3DDevice9::EndScene)
    // 'detour'   : address of your replacement function
    // Returns a callable trampoline (original function) or nullptr on failure.
    void* trampoline(void* target, void* detour);

    // Removes the hook installed on 'target'.
    bool uninstall(void* target);
}