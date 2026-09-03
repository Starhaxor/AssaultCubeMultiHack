#pragma once

// ============================================================================
// HACK DOSYASI (hacks.h / hacks.cpp)
// ----------------------------------------------------------------------------
// YENI HACK EKLEMEK ISTEDIGINDE YAPACAGIN 3 SEY:
//
//   1. menu_state.h/.cpp : acma/kapama degiskenini ekle (örn: bool bFly)
//   2. menu_sections.cpp : ilgili bolume ImGui::Checkbox satirini ekle
//   3. hacks.cpp         : asagidaki "HACK'LER" bolumune calistiran kodu yaz
//
// Calisma sekli:
//   - Worker (arka plan)  : her 10ms'de bir Tick() calisir. Surekli acik
//                           kalmasi gereken seyler buradadir (yama, fly).
//                           Ornek: RapidFireTick(), FlyTick()
//   - Kapaninca bir kez    : tek seferlik isler icin (örn: isinlanma tusu)
//   - Her frame            : RunAimbot() ve RenderOverlay() her karede
//                           cagrilir (ESP cizimi, aimbot burada).
// ============================================================================

// Kod yamalari (ammo NOP, godmode/onehit kancasi).
// Hepsi beklenen byte'lari once dogrular; surum tutmazsa reddeder.
namespace patches
{
	bool SetAmmo(bool enable);
	bool SetGodMode(bool enable);
	bool SetOneHit(bool enable);

	bool AmmoActive();
	bool GodActive();
	bool OneHitActive();

	void Shutdown(); // hepsini geri al (cikisda cagrilir)
}

// Ozellik motoru: arka plan iscisi + her-kare aimbot ve overlay.
namespace hacks
{
	bool Start(); // isci baslat
	void Stop();  // isci durdur (cikisda cagrilir)

	void RunAimbot();     // her karede bir kez cagir
	void RenderOverlay(); // ImGui cercevesi icinde, EndFrame'den once cagir
	int CamIndex();       // kilitli projeksiyon no (tani icin)
}
