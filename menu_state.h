#pragma once

// ============================================================================
// AYAR DEGISKENLERI BURADA (menu_state.h / menu_state.cpp)
// ----------------------------------------------------------------------------
// Menudeki her acma/kapama dugmesinin karsiligi olan degisken buradadir.
// Yeni hack eklerken: once buraya degiskenini ekle (örn: bool bFly),
// sonra menu_sections.cpp'de checkbox'ini, sonra hacks.cpp'de kodunu yaz.
// ============================================================================

namespace menu
{
	extern bool show;

	// Player / weapon: verified code patches, work on this build.
	extern bool bGodMode;
	extern bool bOneHit;
	extern bool bUnlimitedAmmo;
	extern bool bRapidFire;
	extern int rapidRate;

	// Aimbot / ESP / movement need entity offsets (offsets.h).
	// They stay disabled in the UI until those are known.
	extern bool bAimbot;
	extern bool bTeamCheck;
	extern float fovScale;
	extern float aimSmooth;

	extern bool bESP;
	extern bool bEspBox;
	extern bool bEspLines;
	extern bool bEspName;
	extern bool bEspHealth;
	extern float espHeight;

	extern bool bCrosshair;
	extern float crosshairSize;
	extern float crosshairColor[3];

	extern bool bFly;
	extern float flySpeed;
	extern bool bSpeedHack;
	extern float speedValue;
	extern bool bHasTeleport;
	extern float savedPos[3];
	extern float savedFeet[3];
}
