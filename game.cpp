#include "pch.h"

#include <cmath>
#include <cstring>

#include "game.h"
#include "offsets.h"

namespace game
{
	bool DetectPosMap(uintptr_t ent, uintptr_t outMap[3]);
	int DetectPosMaps(uintptr_t ent, uintptr_t outMaps[4][3]);

	uintptr_t Base()
	{
		return reinterpret_cast<uintptr_t>(GetModuleHandleA(offsets::MODULE));
	}

	bool OutputReady()
	{
		return Base() != 0 && LocalPlayer() != 0;
	}

	bool EntityConfigured()
	{
		return offsets::ENTITYLIST_PTR != 0 &&
			offsets::OFF_HEAD_X != 0 &&
			offsets::OFF_YAW != 0 &&
			offsets::OFF_PITCH != 0;
	}

	bool EntityReady()
	{
		if (!EntityConfigured())
			return false;
		uintptr_t local = LocalPlayer();
		if (!local)
			return false;
		float hp = 0.0f;
		if (!ReadHealth(local, hp) || hp < 1.0f || hp > 100.0f)
			return false;
		return EntityList() != 0;
	}

	bool ReadU32(uintptr_t addr, uint32_t& out)
	{
		if (!addr) return false;
		__try { out = *reinterpret_cast<volatile uint32_t*>(addr); return true; }
		__except (EXCEPTION_EXECUTE_HANDLER) { return false; }
	}

	bool ReadFloat(uintptr_t addr, float& out)
	{
		if (!addr) return false;
		__try { out = *reinterpret_cast<volatile float*>(addr); return true; }
		__except (EXCEPTION_EXECUTE_HANDLER) { return false; }
	}

	bool ReadBytes(uintptr_t addr, void* out, size_t size)
	{
		if (!addr || !out || !size) return false;
		__try { std::memcpy(out, reinterpret_cast<void*>(addr), size); return true; }
		__except (EXCEPTION_EXECUTE_HANDLER) { return false; }
	}

	bool WriteU32(uintptr_t addr, uint32_t value)
	{
		if (!addr) return false;
		__try { *reinterpret_cast<volatile uint32_t*>(addr) = value; return true; }
		__except (EXCEPTION_EXECUTE_HANDLER) { return false; }
	}

	bool WriteFloat(uintptr_t addr, float value)
	{
		if (!addr) return false;
		__try { *reinterpret_cast<volatile float*>(addr) = value; return true; }
		__except (EXCEPTION_EXECUTE_HANDLER) { return false; }
	}

	bool WriteBytes(uintptr_t addr, const void* data, size_t size)
	{
		if (!addr || !data || !size) return false;
		__try { std::memcpy(reinterpret_cast<void*>(addr), data, size); return true; }
		__except (EXCEPTION_EXECUTE_HANDLER) { return false; }
	}

	uintptr_t LocalPlayer()
	{
		uintptr_t base = Base();
		if (!base)
			return 0;
		uint32_t ptr = 0;
		if (!ReadU32(base + offsets::LOCALPLAYER_PTR, ptr) || !ptr)
			return 0;
		return static_cast<uintptr_t>(ptr);
	}

	uintptr_t EntityList()
	{
		uintptr_t base = Base();
		if (!base || !offsets::ENTITYLIST_PTR)
			return 0;
		uint32_t ptr = 0;
		if (!ReadU32(base + offsets::ENTITYLIST_PTR, ptr) || !ptr)
			return 0;
		return static_cast<uintptr_t>(ptr);
	}

	int PlayerCount()
	{
		uintptr_t base = Base();
		if (!base || !offsets::PLAYERCOUNT)
			return offsets::MAX_PLAYERS;
		uint32_t n = 0;
		if (!ReadU32(base + offsets::PLAYERCOUNT, n) || n == 0 || n > 64)
			return offsets::MAX_PLAYERS;
		if (n > static_cast<uint32_t>(offsets::MAX_PLAYERS))
			return offsets::MAX_PLAYERS;
		return static_cast<int>(n);
	}

	bool ReadHealth(uintptr_t ent, float& hp)
	{
		// Health is an INT in the engine (100 as float reads ~0).
		uint32_t h = 0;
		if (!ReadU32(ent + offsets::OFF_HEALTH, h))
			return false;
		hp = static_cast<float>(static_cast<int32_t>(h));
		return true;
	}

	bool ReadHead(uintptr_t ent, float out[3])
	{
		return ReadFloat(ent + offsets::OFF_HEAD_X, out[0]) &&
			ReadFloat(ent + offsets::OFF_HEAD_Y, out[1]) &&
			ReadFloat(ent + offsets::OFF_HEAD_Z, out[2]);
	}

	bool ReadPos(uintptr_t ent, float out[3])
	{
		uintptr_t map[3] = {};
		if (!DetectPosMap(ent, map))
			return false;
		return ReadFloat(ent + map[0], out[0]) &&
			ReadFloat(ent + map[1], out[1]) &&
			ReadFloat(ent + map[2], out[2]);
	}

	bool WriteHead(uintptr_t ent, const float in[3])
	{
		return WriteFloat(ent + offsets::OFF_HEAD_X, in[0]) &&
			WriteFloat(ent + offsets::OFF_HEAD_Y, in[1]) &&
			WriteFloat(ent + offsets::OFF_HEAD_Z, in[2]);
	}

	bool WritePos(uintptr_t ent, const float in[3])
	{
		uintptr_t maps[4][3] = {};
		int n = DetectPosMaps(ent, maps);
		if (n <= 0)
			return false;
		// Write every feet copy (o, newpos, ...) so physics can't snap back.
		for (int i = 0; i < n; ++i)
		{
			WriteFloat(ent + maps[i][0], in[0]);
			WriteFloat(ent + maps[i][1], in[1]);
			WriteFloat(ent + maps[i][2], in[2]);
		}
		return true;
	}

	bool GetPosMap(uintptr_t outAddrs[3])
	{
		uintptr_t local = LocalPlayer();
		if (!local)
			return false;
		uintptr_t map[3] = {};
		if (!DetectPosMap(local, map))
			return false;
		outAddrs[0] = map[0]; outAddrs[1] = map[1]; outAddrs[2] = map[2];
		return true;
	}

	bool ReadAngles(float& yaw, float& pitch)
	{
		uintptr_t local = LocalPlayer();
		if (!local)
			return false;
		return ReadFloat(local + offsets::OFF_YAW, yaw) &&
			ReadFloat(local + offsets::OFF_PITCH, pitch);
	}

	bool ReadFov(float& fov, uint32_t& raw)
	{
		uintptr_t base = Base();
		if (!base || !offsets::FOV)
			return false;
		if (!ReadU32(base + offsets::FOV, raw))
			return false;
		float f = 0.0f;
		std::memcpy(&f, &raw, 4);
		if (f > 20.0f && f < 170.0f)
		{
			fov = f;
			return true;
		}
		uint32_t asInt = raw;
		if (asInt >= 20 && asInt <= 170)
		{
			fov = static_cast<float>(asInt);
			return true;
		}
		return false;
	}

	// Feet/origin layout auto-detection. Scans the whole player struct for
	// vec3 triples sitting right below the head (all of o/newpos copies).
	// Returns how many matching triples were found (0 = unknown).
	int DetectPosMaps(uintptr_t ent, uintptr_t outMaps[4][3])
	{
		float h[3] = {};
		if (!ReadHead(ent, h))
			return 0;
		int found = 0;
		for (uintptr_t off = 0x10; off <= 0x200 && found < 4; off += 4)
		{
			float p[3] = {};
			if (!ReadFloat(ent + off, p[0]) ||
				!ReadFloat(ent + off + 4, p[1]) ||
				!ReadFloat(ent + off + 8, p[2]))
				continue;
			bool sane = true;
			for (int i = 0; i < 3; ++i)
			{
				if (!std::isfinite(p[i]) || std::fabs(p[i]) > 1e5f)
				{
					sane = false;
					break;
				}
			}
			if (!sane)
				continue;
			if (std::fabs(p[0] - h[0]) > 1.5f || std::fabs(p[1] - h[1]) > 1.5f)
				continue;
			float dz = h[2] - p[2];
			if (dz < 1.0f || dz > 60.0f)
				continue;
			outMaps[found][0] = off;
			outMaps[found][1] = off + 4;
			outMaps[found][2] = off + 8;
			++found;
		}
		return found;
	}

	// Feet/origin layout auto-detection. Tries candidate (x,y,z) offset
	// triples and keeps the one sitting right below the head.
	bool DetectPosMap(uintptr_t ent, uintptr_t outMap[3])
	{
		uintptr_t maps[4][3] = {};
		if (DetectPosMaps(ent, maps) <= 0)
			return false;
		outMap[0] = maps[0][0];
		outMap[1] = maps[0][1];
		outMap[2] = maps[0][2];
		return true;
	}
}
