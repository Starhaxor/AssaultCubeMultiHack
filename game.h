#pragma once

#include <cstdint>

// ============================================================================
// GUVENLI BELLEK ERISIMI (game.h / game.cpp)
// ----------------------------------------------------------------------------
// Oyunun hafizasini OKUMAK/YAZMAK icin kullanacagin tum fonksiyonlar burada.
// Hepsi SEH korumalidir: yanlis adres versen bile oyun COKMEZ, sadece false
// doner. Yeni bir deger okuman gerektiginde hazir sablon:
//
//   float deger = 0.0f;
//   if (game::ReadFloat(adres, deger)) { ...kullan... }
//   game::WriteFloat(adres, yeniDeger); // yazma
//
// Oyuncu listesi, can, kafa konumu gibi oyun veri yardimcilari da burada.
// ============================================================================
namespace game
{
	uintptr_t Base(); // ac_client.exe module base, 0 if not loaded

	// Verified systems usable (base + localplayer pointer present).
	bool OutputReady();

	// Full entity system configured (offsets filled in offsets.h).
	bool EntityConfigured();

	// Entity system configured AND live pointers validate right now.
	bool EntityReady();

	bool ReadU32(uintptr_t addr, uint32_t& out);
	bool ReadFloat(uintptr_t addr, float& out);
	bool ReadBytes(uintptr_t addr, void* out, size_t size);
	bool WriteU32(uintptr_t addr, uint32_t value);
	bool WriteFloat(uintptr_t addr, float value);
	bool WriteBytes(uintptr_t addr, const void* data, size_t size);

	uintptr_t LocalPlayer();  // validated or 0
	uintptr_t EntityList();   // validated or 0
	int PlayerCount();        // clamped 0..MAX_PLAYERS
	bool ReadHealth(uintptr_t ent, float& hp);
	bool ReadHead(uintptr_t ent, float out[3]); // world xyz, z up
	bool ReadPos(uintptr_t ent, float out[3]);  // feet xyz (auto-detected)
	bool WriteHead(uintptr_t ent, const float in[3]);
	bool WritePos(uintptr_t ent, const float in[3]); // no-op if undetected
	bool ReadAngles(float& yaw, float& pitch); // degrees
	bool ReadFov(float& fov, uint32_t& raw); // game FOV cvar + raw bits
	bool GetPosMap(uintptr_t outAddrs[3]); // detected feet mapping (debug)
	int DetectPosMaps(uintptr_t ent, uintptr_t outMaps[4][3]); // all feet candidates
}
