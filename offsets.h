#pragma once

#include <cstdint>

// ============================================================================
// ADRES TABLOSU BURADA (offsets.h) - AssaultCube v1.3.0.2
// ----------------------------------------------------------------------------
// Oyunun tum bellek adresleri TEK dosyada toplanmistir.
// Surum degisirse ya da yeni adres bulursan SADECE burayi guncelle,
// kodun geri kalanina dokunmana gerek yok.
// ============================================================================
//
// Verified live in-game (crosshair-on-enemy calibration):
//   - Head is a plain vec3: X=0x4, Y=0x8, Z=0xC (z up). The public tables
//     mislabel Y/Z; trust the live test, not the labels.
//   - Forward = (sin(yaw)cos(p), -cos(yaw)cos(p), sin(p)), RH basis with
//     right = up x forward. Yaw 0 = North(-Y), 90 = East(+X).
//   - Patches/localplayer/health from AssaultCubeConsoleHack_1 (same build).
//
// Feet/origin layout is auto-detected at runtime (game.cpp) by matching
// against head, so teleport/fly never depend on guessed offsets.
namespace offsets
{
	constexpr const char* MODULE = "ac_client.exe";

	// dec [eax] -> nop nop (unlimited ammo). Expects bytes FF 08.
	constexpr uintptr_t AMMO_PATCH = 0xC73EF;

	// sub [ebx+04],esi / mov eax,esi (damage handler for godmode/onehit).
	// Expects bytes 29 73 04 8B C6.
	constexpr uintptr_t HEALTH_HOOK = 0x1C223;

	// Pointer to local playerent. Health = localplayer + OFF_HEALTH.
	constexpr uintptr_t LOCALPLAYER_PTR = 0x17E0A8;

	// Entity list: array of playerent pointers. Count next to it.
	constexpr uintptr_t ENTITYLIST_PTR = 0x18AC04;
	constexpr uintptr_t PLAYERCOUNT = 0x18AC0C;

	// Game FOV cvar (float, e.g. 90.0).
	constexpr uintptr_t FOV = 0x18A7CC;

	// View matrix address (UNUSED: proven bogus, projection is camera-based).
	constexpr uintptr_t VIEWMATRIX = 0;

	// playerent fields (verified).
	constexpr uintptr_t OFF_HEALTH = 0xEC; // int
	constexpr uintptr_t OFF_ARMOR = 0xF0;  // int
	constexpr uintptr_t OFF_TEAM = 0x30C;  // 0 = CLA, 1 = RVSF
	constexpr uintptr_t OFF_NAME = 0x205;  // char[16]

	// Head/world position, plain vec3 (verified).
	constexpr uintptr_t OFF_HEAD_X = 0x4;
	constexpr uintptr_t OFF_HEAD_Y = 0x8;
	constexpr uintptr_t OFF_HEAD_Z = 0xC;

	// View angles, degrees (verified live).
	constexpr uintptr_t OFF_YAW = 0x34;
	constexpr uintptr_t OFF_PITCH = 0x38;

	// Weapon cooldowns: writing 0 = instant refire (rapid fire).
	constexpr uintptr_t OFF_FAST_AR = 0x164;
	constexpr uintptr_t OFF_FAST_SNIPER = 0x160;
	constexpr uintptr_t OFF_FAST_SHOTGUN = 0x158;

	constexpr int MAX_PLAYERS = 32;
}
