#include "pch.h"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <vector>

#include "imgui.h"

#include "hacks.h"
#include "game.h"
#include "menu_state.h"
#include "offsets.h"

namespace
{
	volatile bool s_run = false;
	HANDLE s_thread = nullptr;

	constexpr float kDeg2Rad = 0.01745329f;
	constexpr float kRad2Deg = 57.29578f;

	int s_cam = 1; // locked forward hypothesis (live-calibrated, see offsets.h)

	// Verified forward: f = (sin(yaw)cos(p), -cos(yaw)cos(p), sin(p)).
	// Screen basis (verified east-on-right): right = up x fwd, up = fwd x right.
	void TrueForward(float yr, float pr, float f[3])
	{
		f[0] = std::sin(yr) * std::cos(pr);
		f[1] = -std::cos(yr) * std::cos(pr);
		f[2] = std::sin(pr);
	}

	float NormalizeYaw(float yaw)
	{
		while (yaw > 180.0f) yaw -= 360.0f;
		while (yaw < -180.0f) yaw += 360.0f;
		return yaw;
	}

	bool ProjectWith(const float fwd[3], const float cam[3], const float in[3],
		float tanHalfX, float tanHalfY, ImVec2 disp, float& x, float& y)
	{
		// right = worldUp x fwd (east-on-right verified), up = fwd x right.
		float rx = -fwd[1], ry = fwd[0], rz = 0.0f;
		float rlen = std::sqrt(rx * rx + ry * ry);
		if (rlen < 1e-6f)
			return false;
		rx /= rlen; ry /= rlen;
		float ux = fwd[1] * rz - fwd[2] * ry;
		float uy = fwd[2] * rx - fwd[0] * rz;
		float uz = fwd[0] * ry - fwd[1] * rx;

		float dx = in[0] - cam[0];
		float dy = in[1] - cam[1];
		float dz = in[2] - cam[2];
		float depth = dx * fwd[0] + dy * fwd[1] + dz * fwd[2];
		if (depth < 0.5f)
			return false;
		float sx = dx * rx + dy * ry + dz * rz;
		float sy = dx * ux + dy * uy + dz * uz;
		x = disp.x * 0.5f + (sx / depth) / tanHalfX * (disp.x * 0.5f);
		y = disp.y * 0.5f - (sy / depth) / tanHalfY * (disp.y * 0.5f);
		return true;
	}

	bool IsGameForeground()
	{
		HWND fg = GetForegroundWindow();
		if (!fg)
			return false;
		DWORD pid = 0;
		GetWindowThreadProcessId(fg, &pid);
		return pid == GetCurrentProcessId();
	}

	// Matrix-free projection: verified forward + basis (see offsets.h).
	bool WorldToScreen(const float in[3], float& x, float& y)
	{
		uintptr_t local = game::LocalPlayer();
		if (!local)
			return false;
		float yaw = 0.0f, pitch = 0.0f, head[3] = {};
		if (!game::ReadAngles(yaw, pitch) || !game::ReadHead(local, head))
			return false;
		float fov = 75.0f;
		uint32_t fovRaw = 0;
		game::ReadFov(fov, fovRaw); // fallback 75 on failure
		(void)fovRaw;

		ImVec2 disp = ImGui::GetIO().DisplaySize;
		if (disp.x <= 0.0f || disp.y <= 0.0f)
			return false;
		float tanHalfY = std::tan(fov * 0.5f * kDeg2Rad);
		if (tanHalfY < 0.05f)
			return false;
		float tanHalfX = tanHalfY * (disp.x / disp.y);

		float fwd[3] = {};
		TrueForward(yaw * kDeg2Rad, pitch * kDeg2Rad, fwd);
		return ProjectWith(fwd, head, in, tanHalfX, tanHalfY, disp, x, y);
	}

	// Iterates valid enemy entities. Stops at first invalid list (SEH-safe).
	// Team check fails OPEN: unknown/garbage team values never filter.
	template <typename Fn>
	void ForEachTarget(Fn fn)
	{
		uintptr_t local = game::LocalPlayer();
		uintptr_t list = game::EntityList();
		if (!local || !list)
			return;

		float localHp = 0.0f;
		if (!game::ReadHealth(local, localHp) || localHp < 1.0f)
			return;

		uint32_t localTeam = 0;
		bool haveTeams = false;
		if (offsets::OFF_TEAM &&
			game::ReadU32(local + offsets::OFF_TEAM, localTeam) && localTeam <= 1)
			haveTeams = true;

		float localHead[3] = {};
		if (!game::ReadHead(local, localHead))
			return;

		int count = game::PlayerCount();
		for (int i = 0; i < count; ++i)
		{
			uint32_t ent = 0;
			if (!game::ReadU32(list + static_cast<uintptr_t>(i) * 4, ent) || !ent)
				continue;
			uintptr_t e = static_cast<uintptr_t>(ent);
			if (e == local)
				continue;
			float hp = 0.0f;
			if (!game::ReadHealth(e, hp) || hp < 1.0f || hp > 100.0f)
				continue;
			if (menu::bTeamCheck && haveTeams && offsets::OFF_TEAM)
			{
				uint32_t team = 0;
				if (game::ReadU32(e + offsets::OFF_TEAM, team) && team <= 1 && team == localTeam)
					continue;
			}
			float head[3] = {};
			if (!game::ReadHead(e, head))
				continue;
			fn(e, hp, head, localHead);
		}
	}

	void RapidFireTick()
	{
		if (!menu::bRapidFire || menu::show || !IsGameForeground())
			return;
		// Memory fast-fire: zero the weapon cooldowns (SEH-safe data writes).
		if (uintptr_t local = game::LocalPlayer())
		{
			if (offsets::OFF_FAST_AR) game::WriteFloat(local + offsets::OFF_FAST_AR, 0.0f);
			if (offsets::OFF_FAST_SNIPER) game::WriteFloat(local + offsets::OFF_FAST_SNIPER, 0.0f);
			if (offsets::OFF_FAST_SHOTGUN) game::WriteFloat(local + offsets::OFF_FAST_SHOTGUN, 0.0f);
		}
		// Auto-clicker on top: helps semi-auto weapons.
		if (!(GetAsyncKeyState(VK_LBUTTON) & 0x8000))
			return;
		mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
		Sleep(menu::rapidRate > 0 ? static_cast<DWORD>(menu::rapidRate) : 80);
		mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
	}

	void FlyTick()
	{
		if (!menu::bFly || !game::EntityReady())
			return;
		uintptr_t local = game::LocalPlayer();
		if (!local)
			return;
		// Direct Z control every tick beats gravity (head + auto feet).
		float step = menu::flySpeed * 0.15f;
		float head[3] = {}, feet[3] = {};
		if (!game::ReadHead(local, head) || !game::ReadPos(local, feet))
			return;
		if (GetAsyncKeyState(VK_SPACE) & 0x8000)
		{
			head[2] += step;
			feet[2] += step;
			game::WriteHead(local, head);
			game::WritePos(local, feet);
		}
		else if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
		{
			head[2] -= step;
			feet[2] -= step;
			game::WriteHead(local, head);
			game::WritePos(local, feet);
		}
		else
		{
			// Hover: pin Z so we neither sink nor rise.
			game::WriteHead(local, head);
			game::WritePos(local, feet);
		}
	}

	// ========================================================================
	// ARKA PLAN ISLERI (her 10ms'de bir calisir)
	// Surekli acik kalmasi gereken hack'ler buraya: yama uygulama,
	// rapid fire tiklama, fly yukseklik sabitleme.
	// Yeni "surekli" hack eklerken: ...Tick() fonksiyonu yaz, Worker'dan cagir.
	// ========================================================================
	DWORD WINAPI Worker(LPVOID)
	{
		while (s_run)
		{
			// Enforce code patches; revert the toggle if the version refuses.
			if (menu::bUnlimitedAmmo && !patches::SetAmmo(true))
				menu::bUnlimitedAmmo = false;
			if (!menu::bUnlimitedAmmo && patches::AmmoActive())
				patches::SetAmmo(false);

			if (menu::bGodMode && !patches::SetGodMode(true))
				menu::bGodMode = false;
			if (!menu::bGodMode && patches::GodActive())
				patches::SetGodMode(false);

			if (menu::bOneHit && !patches::SetOneHit(true))
				menu::bOneHit = false;
			if (!menu::bOneHit && patches::OneHitActive())
				patches::SetOneHit(false);

			RapidFireTick();
			FlyTick();

			Sleep(10);
		}
		return 0;
	}
}

// ============================================================================
// KOD YAMALARI (eskiden patches.cpp idi)
// Ammo NOP + godmode/onehit kancasi. Hepsi beklenen byte'i once dogrular.
// ============================================================================
namespace
{
	bool g_ammoOn = false;

	uint8_t g_healthOrig[5] = {};
	uint8_t* g_cave = nullptr;
	uint8_t* g_godFlag = nullptr;
	uint8_t* g_oneHitFlag = nullptr;
	bool g_godOn = false;
	bool g_oneHitOn = false;

	const uint8_t kAmmoExpect[2] = { 0xFF, 0x08 };
	const uint8_t kAmmoNop[2] = { 0x90, 0x90 };
	const uint8_t kHealthExpect[5] = { 0x29, 0x73, 0x04, 0x8B, 0xC6 };

	bool PatchCode(uintptr_t addr, const void* data, size_t size)
	{
		DWORD old = 0;
		if (!VirtualProtect(reinterpret_cast<void*>(addr), size, PAGE_EXECUTE_READWRITE, &old))
			return false;
		bool ok = game::WriteBytes(addr, data, size);
		VirtualProtect(reinterpret_cast<void*>(addr), size, old, &old);
		FlushInstructionCache(GetCurrentProcess(), reinterpret_cast<LPCVOID>(addr), size);
		return ok;
	}

	void AppendI32(std::vector<uint8_t>& code, int32_t v)
	{
		uint8_t* b = reinterpret_cast<uint8_t*>(&v);
		code.insert(code.end(), b, b + 4);
	}

	// Dogrulanmis dis hack'teki cave'in birebiri. Hata olursa false doner.
	bool InstallHealthHook(uintptr_t base)
	{
		if (g_cave)
			return true;

		uintptr_t hook = base + offsets::HEALTH_HOOK;
		uintptr_t localPtrAddr = base + offsets::LOCALPLAYER_PTR;
		uintptr_t ret = hook + 5;

		uint8_t cur[5] = {};
		if (!game::ReadBytes(hook, cur, sizeof(cur)) ||
			std::memcmp(cur, kHealthExpect, sizeof(cur)) != 0)
			return false; // yanlis surum: reddet
		std::memcpy(g_healthOrig, cur, sizeof(cur));

		g_cave = static_cast<uint8_t*>(VirtualAlloc(nullptr, 512, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
		g_godFlag = static_cast<uint8_t*>(VirtualAlloc(nullptr, 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
		g_oneHitFlag = static_cast<uint8_t*>(VirtualAlloc(nullptr, 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
		if (!g_cave || !g_godFlag || !g_oneHitFlag)
		{
			if (g_cave) VirtualFree(g_cave, 0, MEM_RELEASE);
			if (g_godFlag) VirtualFree(g_godFlag, 0, MEM_RELEASE);
			if (g_oneHitFlag) VirtualFree(g_oneHitFlag, 0, MEM_RELEASE);
			g_cave = g_godFlag = g_oneHitFlag = nullptr;
			return false;
		}
		*g_godFlag = 0;
		*g_oneHitFlag = 0;

		uintptr_t caveBase = reinterpret_cast<uintptr_t>(g_cave);
		std::vector<uint8_t> code;

		code.push_back(0x50); // push eax
		code.push_back(0xA1); // mov eax,[localplayerPtr]
		AppendI32(code, static_cast<int32_t>(localPtrAddr));
		code.push_back(0x05); // add eax,0xE8 (ebx == local+E8 => [ebx+4] candir)
		AppendI32(code, 0xE8);
		code.push_back(0x39); code.push_back(0xC3); // cmp ebx,eax
		code.push_back(0x0F); code.push_back(0x85); // jne enemy
		size_t jnePos = code.size(); AppendI32(code, 0);

		// local: cmp byte [god],0
		code.push_back(0x80); code.push_back(0x3D);
		AppendI32(code, static_cast<int32_t>(reinterpret_cast<uintptr_t>(g_godFlag)));
		code.push_back(0x00);
		code.push_back(0x74); // je localNormal
		size_t jeLocalPos = code.size(); code.push_back(0x00);

		code.push_back(0x58); // pop eax (god: sub'u atla)
		code.push_back(0x8B); code.push_back(0xC6); // mov eax,esi
		code.push_back(0xE9); // jmp ret
		size_t godRetPos = code.size(); AppendI32(code, 0);

		size_t localNormalPos = code.size();
		code.push_back(0x58); // pop eax
		code.push_back(0x29); code.push_back(0x73); code.push_back(0x04); // sub [ebx+4],esi
		code.push_back(0x8B); code.push_back(0xC6); // mov eax,esi
		code.push_back(0xE9); // jmp ret
		size_t localRetPos = code.size(); AppendI32(code, 0);

		size_t enemyPos = code.size();
		code.push_back(0x80); code.push_back(0x3D); // cmp byte [onehit],0
		AppendI32(code, static_cast<int32_t>(reinterpret_cast<uintptr_t>(g_oneHitFlag)));
		code.push_back(0x00);
		code.push_back(0x74); // je enemyNormal
		size_t jeEnemyPos = code.size(); code.push_back(0x00);

		code.push_back(0x58); // pop eax
		code.push_back(0xC7); code.push_back(0x43); code.push_back(0x04); // mov [ebx+4],0
		code.push_back(0x00); code.push_back(0x00); code.push_back(0x00); code.push_back(0x00);
		code.push_back(0x8B); code.push_back(0xC6); // mov eax,esi
		code.push_back(0xE9); // jmp ret
		size_t oneHitRetPos = code.size(); AppendI32(code, 0);

		size_t enemyNormalPos = code.size();
		code.push_back(0x58); // pop eax
		code.push_back(0x29); code.push_back(0x73); code.push_back(0x04); // sub [ebx+4],esi
		code.push_back(0x8B); code.push_back(0xC6); // mov eax,esi
		code.push_back(0xE9); // jmp ret
		size_t enemyRetPos = code.size(); AppendI32(code, 0);

		auto patchRel32 = [&](size_t at, uintptr_t target)
		{
			int32_t rel = static_cast<int32_t>(target - (caveBase + at + 4));
			std::memcpy(&code[at], &rel, 4);
		};
		int32_t jneRel = static_cast<int32_t>((caveBase + enemyPos) - (caveBase + jnePos + 4));
		std::memcpy(&code[jnePos], &jneRel, 4);
		code[jeLocalPos] = static_cast<uint8_t>(localNormalPos - (jeLocalPos + 1));
		code[jeEnemyPos] = static_cast<uint8_t>(enemyNormalPos - (jeEnemyPos + 1));
		patchRel32(godRetPos, ret);
		patchRel32(localRetPos, ret);
		patchRel32(oneHitRetPos, ret);
		patchRel32(enemyRetPos, ret);

		if (!game::WriteBytes(caveBase, code.data(), code.size()))
			return false;
		FlushInstructionCache(GetCurrentProcess(), g_cave, code.size());

		uint8_t jmp[5] = { 0xE9, 0, 0, 0, 0 };
		int32_t rel = static_cast<int32_t>(caveBase - (hook + 5));
		std::memcpy(&jmp[1], &rel, 4);
		return PatchCode(hook, jmp, sizeof(jmp));
	}

	void RemoveHealthHook(uintptr_t base)
	{
		if (!g_cave)
			return;
		PatchCode(base + offsets::HEALTH_HOOK, g_healthOrig, sizeof(g_healthOrig));
		VirtualFree(g_cave, 0, MEM_RELEASE);
		VirtualFree(g_godFlag, 0, MEM_RELEASE);
		VirtualFree(g_oneHitFlag, 0, MEM_RELEASE);
		g_cave = g_godFlag = g_oneHitFlag = nullptr;
	}
}

namespace patches
{
	bool SetAmmo(bool enable)
	{
		uintptr_t base = game::Base();
		if (!base)
			return false;
		if (enable == g_ammoOn)
			return true;

		uintptr_t addr = base + offsets::AMMO_PATCH;
		if (enable)
		{
			uint8_t cur[2] = {};
			if (!game::ReadBytes(addr, cur, sizeof(cur)) ||
				std::memcmp(cur, kAmmoExpect, sizeof(cur)) != 0)
				return false; // yanlis surum: reddet
			if (!PatchCode(addr, kAmmoNop, sizeof(kAmmoNop)))
				return false;
		}
		else
		{
			if (!PatchCode(addr, kAmmoExpect, sizeof(kAmmoExpect)))
				return false;
		}
		g_ammoOn = enable;
		return true;
	}

	bool SetGodMode(bool enable)
	{
		uintptr_t base = game::Base();
		if (!base)
			return false;
		if (enable)
		{
			if (!InstallHealthHook(base))
				return false;
			*g_godFlag = 1;
			g_godOn = true;
			return true;
		}
		g_godOn = false;
		if (g_godFlag) *g_godFlag = 0;
		if (!g_oneHitOn)
			RemoveHealthHook(base);
		return true;
	}

	bool SetOneHit(bool enable)
	{
		uintptr_t base = game::Base();
		if (!base)
			return false;
		if (enable)
		{
			if (!InstallHealthHook(base))
				return false;
			*g_oneHitFlag = 1;
			g_oneHitOn = true;
			return true;
		}
		g_oneHitOn = false;
		if (g_oneHitFlag) *g_oneHitFlag = 0;
		if (!g_godOn)
			RemoveHealthHook(base);
		return true;
	}

	bool AmmoActive() { return g_ammoOn; }
	bool GodActive() { return g_godOn; }
	bool OneHitActive() { return g_oneHitOn; }

	void Shutdown()
	{
		uintptr_t base = game::Base();
		if (g_ammoOn && base)
			PatchCode(base + offsets::AMMO_PATCH, kAmmoExpect, sizeof(kAmmoExpect));
		g_ammoOn = false;
		g_godOn = false;
		g_oneHitOn = false;
		if (base)
			RemoveHealthHook(base);
		else
		{
			if (g_cave) VirtualFree(g_cave, 0, MEM_RELEASE);
			if (g_godFlag) VirtualFree(g_godFlag, 0, MEM_RELEASE);
			if (g_oneHitFlag) VirtualFree(g_oneHitFlag, 0, MEM_RELEASE);
			g_cave = g_godFlag = g_oneHitFlag = nullptr;
		}
	}
}

namespace hacks
{
	bool Start()
	{
		if (s_thread)
			return true;
		s_run = true;
		s_thread = CreateThread(nullptr, 0, &Worker, nullptr, 0, nullptr);
		return s_thread != nullptr;
	}

	void Stop()
	{
		s_run = false;
		s_thread = nullptr; // fire-and-forget; worker exits on its own
	}

	int CamIndex()
	{
		return s_cam;
	}

	// ========================================================================
	// HER KARE CALISANLAR (render_hooks.cpp'den cagrilir)
	// Ekrana cizim yapan (ESP, crosshair) ve her kare hesap isteyen
	// (aimbot) hack'ler buradadir.
	// ========================================================================
	void RunAimbot()
	{
		if (!menu::bAimbot || menu::show || !game::EntityReady())
			return;
		if (!(GetAsyncKeyState(VK_RBUTTON) & 0x8000))
			return; // hold right mouse to aim

		uintptr_t local = game::LocalPlayer();
		if (!local)
			return;
		float localHead[3] = {};
		if (!game::ReadHead(local, localHead))
			return;
		float curYaw = 0.0f, curPitch = 0.0f;
		if (!game::ReadFloat(local + offsets::OFF_YAW, curYaw) ||
			!game::ReadFloat(local + offsets::OFF_PITCH, curPitch))
			return;

		uintptr_t best = 0;
		float bestScore = menu::fovScale;
		float bestYaw = 0.0f, bestPitch = 0.0f;

		// Inverse of TrueForward: yaw = atan2(dx, -dy), pitch = asin(dz/dist).
		ForEachTarget([&](uintptr_t e, float, const float head[3], const float*)
		{
			float dx = head[0] - localHead[0];
			float dy = head[1] - localHead[1];
			float dz = head[2] - localHead[2];
			float dist = std::sqrt(dx * dx + dy * dy + dz * dz);
			if (dist < 0.001f)
				return;
			float desYaw = atan2f(dx, -dy) * kRad2Deg;
			float desPitch = asinf(dz / dist) * kRad2Deg;
			float dYaw = NormalizeYaw(desYaw - curYaw);
			float dPitch = desPitch - curPitch;
			float score = std::sqrt(dYaw * dYaw + dPitch * dPitch);
			if (score < bestScore)
			{
				bestScore = score;
				bestYaw = desYaw;
				bestPitch = desPitch;
				best = e;
			}
		});

		if (!best)
			return;

		float smooth = menu::aimSmooth < 1.0f ? 1.0f : menu::aimSmooth;
		float newYaw = NormalizeYaw(curYaw + NormalizeYaw(bestYaw - curYaw) / smooth);
		float newPitch = curPitch + (bestPitch - curPitch) / smooth;
		if (newPitch > 89.0f) newPitch = 89.0f;
		if (newPitch < -89.0f) newPitch = -89.0f;
		game::WriteFloat(local + offsets::OFF_YAW, newYaw);
		game::WriteFloat(local + offsets::OFF_PITCH, newPitch);
	}

	void RenderOverlay()
	{
		// Crosshair: no offsets needed, always available.
		if (menu::bCrosshair && !menu::show)
		{
			ImVec2 disp = ImGui::GetIO().DisplaySize;
			ImVec2 c(disp.x * 0.5f, disp.y * 0.5f);
			float s = menu::crosshairSize;
			ImU32 col = ImGui::ColorConvertFloat4ToU32(ImVec4(
				menu::crosshairColor[0], menu::crosshairColor[1], menu::crosshairColor[2], 1.0f));
			ImDrawList* dl = ImGui::GetBackgroundDrawList();
			dl->AddLine(ImVec2(c.x - s, c.y), ImVec2(c.x + s, c.y), col, 1.5f);
			dl->AddLine(ImVec2(c.x, c.y - s), ImVec2(c.x, c.y + s), col, 1.5f);
		}

		if (!menu::bESP || !game::EntityReady())
			return;

		ImVec2 disp = ImGui::GetIO().DisplaySize;
		ImDrawList* dl = ImGui::GetBackgroundDrawList();
		const ImU32 enemyCol = IM_COL32(255, 70, 60, 255);
		const ImU32 teamCol = IM_COL32(90, 220, 120, 255);
		const ImU32 unknownCol = IM_COL32(255, 200, 90, 255);

		ForEachTarget([&](uintptr_t e, float hp, const float head[3], const float*)
		{
			float tx = 0.0f, ty = 0.0f;
			if (!WorldToScreen(head, tx, ty))
				return;
			float feet[3] = { head[0], head[1], head[2] - menu::espHeight };
			float bx = 0.0f, by = 0.0f;
			if (!WorldToScreen(feet, bx, by))
				return;
			float h = by - ty;
			if (h <= 0.0f || h > disp.y)
				return;
			float w = h * 0.55f;

			ImU32 col = unknownCol;
			if (offsets::OFF_TEAM)
			{
				uint32_t team = 99, localTeam = 99;
				uintptr_t local = game::LocalPlayer();
				bool okE = game::ReadU32(e + offsets::OFF_TEAM, team);
				bool okL = local && game::ReadU32(local + offsets::OFF_TEAM, localTeam);
				if (okE && okL && team <= 1 && localTeam <= 1)
					col = (team == localTeam) ? teamCol : enemyCol;
			}

			if (menu::bEspBox)
				dl->AddRect(ImVec2(tx - w * 0.5f, ty), ImVec2(tx + w * 0.5f, by), col, 0.0f, 0, 1.5f);
			if (menu::bEspHealth)
			{
				float frac = hp / 100.0f;
				if (frac > 1.0f) frac = 1.0f;
				dl->AddLine(ImVec2(tx - w * 0.5f - 4.0f, by),
					ImVec2(tx - w * 0.5f - 4.0f, by - h * frac),
					IM_COL32(80, 255, 80, 255), 2.5f);
			}
			if (menu::bEspName && offsets::OFF_NAME)
			{
				char name[16] = {};
				if (game::ReadBytes(e + offsets::OFF_NAME, name, sizeof(name) - 1))
				{
					name[sizeof(name) - 1] = '\0';
					dl->AddText(ImVec2(tx - w * 0.5f, ty - 14.0f), col, name);
				}
			}
			if (menu::bEspLines)
				dl->AddLine(ImVec2(disp.x * 0.5f, disp.y), ImVec2(tx, by), col, 1.0f);
		});
	}
}
