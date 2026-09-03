#include "hook.h"

#include <cstring>

namespace
{
    // Minimal x86 (32-bit) length disassembler.
    // Returns the byte length of the instruction at 'p'.
    size_t inst_len(const uint8_t* p)
    {
        const uint8_t* start = p;

        // Prefixes
        for (;;)
        {
            uint8_t b = *p;
            if (b == 0x66 || b == 0x67 || b == 0xF0 || b == 0xF2 || b == 0xF3 ||
                b == 0x2E || b == 0x36 || b == 0x3E || b == 0x26 || b == 0x64 || b == 0x65)
            {
                ++p;
                continue;
            }
            break;
        }

        uint8_t op = *p++;

        // Two-byte opcode map (0x0F ...)
        if (op == 0x0F)
        {
            uint8_t op2 = *p++;
            if (op2 >= 0x80 && op2 <= 0x8F)              // jcc rel32
                return (size_t)(p - start) + 4;

            bool imm8 = (op2 == 0xBA || op2 == 0xC7 || op2 == 0xA3 || op2 == 0xA4 ||
                         op2 == 0xA5 || op2 == 0xAB || op2 == 0xAC || op2 == 0x6A ||
                         op2 == 0xA0 || op2 == 0xA1 || op2 == 0xA2 || op2 == 0xA8 ||
                         op2 == 0xA9 || op2 == 0xB3 || op2 == 0xB4 || op2 == 0xB5 ||
                         op2 == 0xB6 || op2 == 0xB7 || op2 == 0xBB || op2 == 0xBC ||
                         op2 == 0xBD || op2 == 0xBE || op2 == 0xBF || op2 == 0xCC ||
                         op2 == 0xD3 || op2 == 0xAE || op2 == 0xAF);
            uint8_t modrm = *p++;
            uint8_t mod = modrm >> 6;
            uint8_t rm = modrm & 7;
            size_t len = (size_t)(p - start);
            if (mod == 3)
                return len + (imm8 ? 1 : 0);
            if (rm == 4) p++;                            // SIB
            if (mod == 0) { if (rm == 5) p += 4; }
            else if (mod == 1) p += 1;
            else if (mod == 2) p += 4;
            return (size_t)(p - start) + (imm8 ? 1 : 0);
        }

        switch (op)
        {
        case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
        case 0x58: case 0x59: case 0x5A: case 0x5B: case 0x5C: case 0x5D: case 0x5E: case 0x5F:
        case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
        case 0x48: case 0x49: case 0x4A: case 0x4B: case 0x4C: case 0x4D: case 0x4E: case 0x4F:
        case 0x90: case 0x98: case 0x99: case 0x9C: case 0x9D: case 0x9E: case 0x9F:
        case 0xC3: case 0xC9: case 0xCC: case 0xF8: case 0xF9: case 0xFC: case 0xFD:
            return (size_t)(p - start);

        case 0xE8: case 0xE9:                           // call/jmp rel32
        case 0x05: case 0x0D: case 0x15: case 0x1D: case 0x25: case 0x2D: case 0x35: case 0x3D:
        case 0x68:
            return (size_t)(p - start) + 4;
        case 0x6A:                                      // push imm8
            return (size_t)(p - start) + 1;

        case 0xEB: case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76:
        case 0x77: case 0x78: case 0x79: case 0x7A: case 0x7B: case 0x7C: case 0x7D: case 0x7E:
        case 0x7F: case 0xE3: case 0xE2: case 0xE1: case 0xE0:
            return (size_t)(p - start) + 1;

        case 0x9A:
            return (size_t)(p - start) + 6;
        case 0xC2: case 0xCA:
            return (size_t)(p - start) + 2;
        case 0xA0: case 0xA1: case 0xA2: case 0xA3:
            return (size_t)(p - start) + 4;
        }

        {
            bool imm8 = false;
            bool imm = false;
            switch (op)
            {
            case 0x80: case 0x82: case 0x83: case 0xC0: case 0xC6:
                imm8 = true; break;
            case 0x81: case 0xC1: case 0xC7:
                imm = true; break;
            }
            uint8_t modrm = *p++;
            uint8_t mod = modrm >> 6;
            uint8_t rm = modrm & 7;
            size_t len = (size_t)(p - start);
            if (mod == 3)
            {
                if (imm8) len += 1;
                if (imm) len += 4;
                return len;
            }
            if (rm == 4) len += 1;                       // SIB
            if (mod == 0) { if (rm == 5) len += 4; }
            else if (mod == 1) { len += 1; }
            else if (mod == 2) { len += 4; }
            if (imm8) len += 1;
            if (imm) len += 4;
            return len;
        }
    }
}

#include <map>

namespace
{
    struct HookContext
    {
        uint8_t* gateway;       // executable memory (VirtualAlloc), stable address
        size_t   gateway_cap;
        uint8_t  saved_bytes[32];
        size_t   gateway_len;
        size_t   saved_len;
        bool     active;
    };

    std::map<uintptr_t, HookContext> g_hooks;

    // Rewrites relative instructions while relocating the stolen bytes from the
    // original address to the gateway, so control flow targets stay correct.
    // Returns the number of bytes actually written to 'dst' (may exceed src_len
    // when short jumps are widened to near jumps).
    size_t relocate_and_copy(uint8_t* dst, const uint8_t* src, size_t src_len)
    {
        size_t in = 0;
        size_t out = 0;
        while (in < src_len)
        {
            size_t ilen = inst_len(src + in);
            uint8_t op = src[in];

            if (op == 0xE8 || op == 0xE9)
            {
                memcpy(dst + out, src + in, ilen);
                int32_t rel;
                memcpy(&rel, src + in + 1, 4);
                uint8_t* target = const_cast<uint8_t*>(src + in + ilen + rel);
                int32_t nrel = (int32_t)(target - (dst + out + ilen));
                memcpy(dst + out + 1, &nrel, 4);
            }
            else if (op == 0x0F && src[in + 1] >= 0x80 && src[in + 1] <= 0x8F)
            {
                memcpy(dst + out, src + in, ilen);
                int32_t rel;
                memcpy(&rel, src + in + 2, 4);
                uint8_t* target = const_cast<uint8_t*>(src + in + ilen + rel);
                int32_t nrel = (int32_t)(target - (dst + out + ilen));
                memcpy(dst + out + 2, &nrel, 4);
            }
            else if (op >= 0x70 && op <= 0x7F)
            {
                // widen short jcc to near jcc (2 -> 6 bytes)
                int8_t rel = (int8_t)src[in + 1];
                uint8_t* target = const_cast<uint8_t*>(src + in + ilen + rel);
                dst[out] = 0x0F;
                dst[out + 1] = (uint8_t)(0x80 + (op - 0x70));
                int32_t nrel = (int32_t)(target - (dst + out + 6));
                memcpy(dst + out + 2, &nrel, 4);
                ilen = 6;
            }
            else if (op == 0xEB)
            {
                // widen short jmp to near jmp (2 -> 5 bytes)
                int8_t rel = (int8_t)src[in + 1];
                uint8_t* target = const_cast<uint8_t*>(src + in + ilen + rel);
                dst[out] = 0xE9;
                int32_t nrel = (int32_t)(target - (dst + out + 5));
                memcpy(dst + out + 1, &nrel, 4);
                ilen = 5;
            }
            else
            {
                memcpy(dst + out, src + in, ilen);
            }

            in += inst_len(src + in);
            out += ilen;
        }
        return out;
    }
}

void* hook::trampoline(void* target, void* detour)
{
    if (!target || !detour) return nullptr;

#ifdef _M_X64
    (void)target; (void)detour;
    return nullptr;
#else
    uint8_t* t = (uint8_t*)target;
    uint8_t* d = (uint8_t*)detour;

    size_t stolen = 0;
    while (stolen < 5)
    {
        size_t l = inst_len(t + stolen);
        if (l == 0 || stolen + l > sizeof(HookContext::saved_bytes))
            return nullptr;
        stolen += l;
    }

    // Gateway must be executable (DEP) and must outlive this call.
    // Old code returned a pointer to a stack-local array -> dangling trampoline,
    // so oEndScene/oReset crashed and the menu never rendered.
    constexpr size_t kGatewayCap = 128;
    uint8_t* gateway = (uint8_t*)VirtualAlloc(nullptr, kGatewayCap,
        MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!gateway)
        return nullptr;

    HookContext ctx{};
    ctx.gateway = gateway;
    ctx.gateway_cap = kGatewayCap;
    ctx.saved_len = stolen;
    ctx.active = false;
    memcpy(ctx.saved_bytes, t, stolen);

    ctx.gateway_len = relocate_and_copy(ctx.gateway, ctx.saved_bytes, stolen);

    // Append jump back: E9 rel32 -> (target + stolen)
    if (ctx.gateway_len + 5 > ctx.gateway_cap)
    {
        VirtualFree(ctx.gateway, 0, MEM_RELEASE);
        return nullptr;
    }
    uint8_t* jb = ctx.gateway + ctx.gateway_len;
    jb[0] = 0xE9;
    int32_t back = (int32_t)((t + stolen) - (jb + 5));
    memcpy(jb + 1, &back, 4);
    ctx.gateway_len += 5;
    FlushInstructionCache(GetCurrentProcess(), ctx.gateway, ctx.gateway_len);

    // Patch target with E9 rel32 -> detour
    DWORD old;
    if (!VirtualProtect(t, stolen, PAGE_EXECUTE_READWRITE, &old))
    {
        VirtualFree(ctx.gateway, 0, MEM_RELEASE);
        return nullptr;
    }
    t[0] = 0xE9;
    int32_t fwd = (int32_t)(d - (t + 5));
    memcpy(t + 1, &fwd, 4);
    for (size_t i = 5; i < stolen; ++i) t[i] = 0x90;
    VirtualProtect(t, stolen, old, &old);
    FlushInstructionCache(GetCurrentProcess(), t, stolen);

    ctx.active = true;
    g_hooks[(uintptr_t)t] = ctx;
    return (void*)g_hooks[(uintptr_t)t].gateway;
#endif
}

bool hook::uninstall(void* target)
{
#ifdef _M_X64
    (void)target;
    return false;
#else
    auto it = g_hooks.find((uintptr_t)target);
    if (it == g_hooks.end() || !it->second.active) return false;

    HookContext& ctx = it->second;
    DWORD old;
    VirtualProtect((uint8_t*)target, ctx.saved_len, PAGE_EXECUTE_READWRITE, &old);
    memcpy((uint8_t*)target, ctx.saved_bytes, ctx.saved_len);
    VirtualProtect((uint8_t*)target, ctx.saved_len, old, &old);
    FlushInstructionCache(GetCurrentProcess(), target, ctx.saved_len);

    ctx.active = false;
    if (ctx.gateway)
        VirtualFree(ctx.gateway, 0, MEM_RELEASE);
    g_hooks.erase(it);
    return true;
#endif
}