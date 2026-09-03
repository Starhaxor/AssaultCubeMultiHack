#include "pch.h"
#include "menu_state.h"

namespace menu
{
	bool show = true;

	bool bGodMode = false;
	bool bOneHit = false;
	bool bUnlimitedAmmo = false;
	bool bRapidFire = false;
	int rapidRate = 80;

	bool bAimbot = false;
	bool bTeamCheck = true;
	float fovScale = 30.0f;
	float aimSmooth = 4.0f;

	bool bESP = false;
	bool bEspBox = true;
	bool bEspLines = false;
	bool bEspName = true;
	bool bEspHealth = true;
	float espHeight = 5.0f;

	bool bCrosshair = false;
	float crosshairSize = 6.0f;
	float crosshairColor[3] = { 1.0f, 0.75f, 0.30f };

	bool bFly = false;
	float flySpeed = 6.0f;
	bool bSpeedHack = false;
	float speedValue = 2.0f;
	bool bHasTeleport = false;
	float savedPos[3] = {};
	float savedFeet[3] = {};
}
