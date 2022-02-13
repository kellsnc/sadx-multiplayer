#include "pch.h"
#include "multiplayer.h"
#include "splitscreen.h"
#include "camera.h"
#include "sound.h"

Trampoline* dsGetVolume_t = nullptr;
Trampoline* dsPlay_timer_v_t = nullptr;
Trampoline* dsPlay_timer_vq_t = nullptr;
Trampoline* dsPlay_oneshot_v_t = nullptr;
Trampoline* dsPlay_Dolby_timer_vq_t = nullptr;

void dsPlay_timer_v_r(int tone, int id, int pri, int volofs, int timer, float x, float y, float z)
{
    if (IsCameraInSphere(x, y, z, 40000.0f))
    {
        int num = SoundQueue_GetOtherThing(tone, (EntityData1*)id); // inlined
        if (num < 0)
        {
            num = SoundQueue_GetFreeIndex(pri); // dsCheckHandle
            if (num < 0)
            {
                return;
            }
            sebuf[num].mode = 1;
            sebuf[num].qnum = -1;
        }
        else
        {
            sebuf[num].mode = 0;
        }

        sebuf[num].timer = timer + 1;
        sebuf[num].pri = pri;
        sebuf[num].id = id;
        sebuf[num].mode |= 0x1130;
        sebuf[num].tone = tone;
        sebuf[num].angle = 0;
        sebuf[num].vol = volofs;
        sebuf[num].volmax = volofs;
        sebuf[num].pitch = 0;
        sebuf[num].pos.x = x;
        sebuf[num].pos.y = y;
        sebuf[num].pos.z = z;
    }
    else
    {
        TARGET_DYNAMIC(dsPlay_timer_v)(tone, id, pri, volofs, timer, x, y, z);
    }
}

void dsPlay_timer_vq_r(int tone, int id, int pri, int volofs, int timer, float x, float y, float z, float rad)
{
    if (SplitScreen::IsActive && IsCameraInSphere(x, y, z, rad))
    {
        int num = SoundQueue_GetOtherThing(tone, (EntityData1*)id); // inlined
        if (num < 0)
        {
            num = SoundQueue_GetFreeIndex(pri); // dsCheckHandle
            if (num < 0)
            {
                return;
            }
            sebuf[num].mode = 1;
            sebuf[num].qnum = -1;
        }
        else
        {
            sebuf[num].mode = 0;
        }

        sebuf[num].timer = timer + 1;
        sebuf[num].pri = pri;
        sebuf[num].id = id;
        sebuf[num].mode |= 0x1130;
        sebuf[num].tone = tone;
        sebuf[num].angle = 0;
        sebuf[num].vol = volofs;
        sebuf[num].volmax = volofs;
        sebuf[num].pitch = 0;
        sebuf[num].pos.x = x;
        sebuf[num].pos.y = y;
        sebuf[num].pos.z = z;
    }
    else
    {
        TARGET_DYNAMIC(dsPlay_timer_vq)(tone, id, pri, volofs, timer, x, y, z, rad);
    }
}

int dsPlay_oneshot_v_r(int tone, int id, int pri, int volofs, float x, float y, float z)
{
    if (SplitScreen::IsActive && IsCameraInSphere(x, y, z, 40000.0f))
    {
        int num = SoundQueue_GetOtherThing(tone, (EntityData1*)id); // inlined
        if (num < 0)
        {
            num = SoundQueue_GetFreeIndex(pri); // dsCheckHandle
            if (num < 0)
            {
                return -1;
            }
            sebuf[num].mode = 1;
            sebuf[num].qnum = -1;
        }
        else
        {
            sebuf[num].mode = 0;
        }

        sebuf[num].timer = 120;
        sebuf[num].pri = -1;
        sebuf[num].id = id;
        sebuf[num].mode |= 0x1130;
        sebuf[num].tone = tone;
        sebuf[num].angle = 0;
        sebuf[num].vol = volofs;
        sebuf[num].volmax = volofs;
        sebuf[num].pitch = 0;
        sebuf[num].pos.x = x;
        sebuf[num].pos.y = y;
        sebuf[num].pos.z = z;
    }
    else
    {
        return TARGET_DYNAMIC(dsPlay_oneshot_v)(tone, id, pri, volofs, x, y, z);
    }
}

void dsPlay_Dolby_timer_vq_r(int tone, int id, int pri, int volofs, int timer, float rad, taskwk* pTaskwk)
{
    if (SplitScreen::IsActive)
    {
        if (pTaskwk && IsCameraInSphere(&pTaskwk->pos, rad))
        {
            int num = SoundQueue_GetOtherThing(tone, (EntityData1*)id); // inlined
            if (num < 0)
            {
                num = SoundQueue_GetFreeIndex(pri); // dsCheckHandle
                if (num < 0)
                {
                    return;
                }
                sebuf[num].mode = 1;
                sebuf[num].qnum = -1;
            }
            else
            {
                sebuf[num].mode = 0;
            }

            sebuf[num].timer = timer + 1;
            sebuf[num].pri = pri;
            sebuf[num].id = id;
            sebuf[num].mode |= 0x1130;
            
            if (Get3Dmode() == 0)
            {
                sebuf[num].mode |= 0x20;
            }
            else
            {
                sebuf[num].mode |= 0x4000;
            }

            sebuf[num].tone = tone;
            sebuf[num].angle = 0;
            sebuf[num].vol = volofs;
            sebuf[num].volmax = volofs;
            sebuf[num].pitch = 0;
            sebuf[num].pos.x = pTaskwk->pos.x;
            sebuf[num].pos.y = pTaskwk->pos.y;
            sebuf[num].pos.z = pTaskwk->pos.z;
            gpDolbyTask[num] = pTaskwk;
        }
    }
    else
    {
        TARGET_DYNAMIC(dsPlay_Dolby_timer_vq)(tone, id, pri, volofs, timer, rad, pTaskwk);
    }
}

static int dsGetVolume_o(int ii)
{
    auto tgt = dsGetVolume_t->Target();
    int ret;
    __asm
    {
        mov eax, [ii]
        call tgt
        mov ret, eax
    }
    return ret;
}

int __cdecl dsGetVolume_r(int ii)
{
    if (SplitScreen::IsActive)
    {
        auto se = &sebuf[ii];
        
        auto pnum = GetTheNearestPlayerNumber(&se->pos);
        float dist = GetDistance(&se->pos, GetCameraPosition(pnum));

        int vol = 0;

        if (dist > 20.0f)
        {
            dist = (dist - 20.0f) * 0.0017241379f;

            if (dist < 0.0f)
            {
                dist = 0.0f;
            }

            vol = (int)-(dist * 127.0f);
        }

        vol += se->vol;

        if (vol < -127)
        {
            vol = -127;
        }

        if (vol > se->volmax)
        {
            return se->volmax;
        }

        return vol;
    }
    else
    {
        return dsGetVolume_o(ii);
    }
}

static void __declspec(naked) dsGetVolume_w()
{
    __asm
    {
        push eax
        call dsGetVolume_r
        add esp, 4
        retn
    }
}

static bool dsDolbySound_r()
{
    if (!SplitScreen::IsActive())
    {
        return false;
    }

    for (int i = 0; i < 36; ++i)
    {
        auto se = &sebuf[i];
        auto twp = gpDolbyTask[i];

        if (se->mode & 0x4000 && twp)
        {
            auto pnum = GetTheNearestPlayerNumber(&twp->pos);
            NJS_VECTOR v = playertwp[pnum]->pos;
            njSubVector(&v, &twp->pos);

            njPushMatrix(0);
            njUnitMatrix(0);
            njRotateY(0, -HIWORD(GetCameraAngle(pnum)));
            njCalcPoint(0, &v, &v);
            njPopMatrix(1);

            v.x *= 0.1f;
            v.y *= 0.1f;
            v.z *= 0.1f;

            Set3DPositionPCM(i, v.x, v.y, v.z);
        }
    }

    return true;
}

static const void* dsDolbySound_o = reinterpret_cast<void*>(0x00424B10);

static void __declspec(naked) dsDolbySound_w()
{
    __asm
    {
        push eax
        call dsDolbySound_r
        test eax, eax
        pop eax
        jnz is_true
        jmp dsDolbySound_o
    is_true:
        retn
    }
}

void dsPlay_oneshot_miles(int tone, int id, int pri, int volofs)
{
    if (multiplayer::IsActive())
    {
        dsPlay_oneshot(tone, id, pri, volofs);
    }
    else
    {
        // Original behaviour:
        if (TASKWK_PLAYERID(gpCharTwp) != 1)
        {
            dsPlay_oneshot(tone, id, pri, volofs);
        }
    }
}

void InitSoundPatches()
{
    dsGetVolume_t = new Trampoline(0x4244A0, 0x4244A7, dsGetVolume_r);
    dsPlay_timer_v_t = new Trampoline(0x424000, 0x424005, dsPlay_timer_v_r);
    dsPlay_timer_vq_t = new Trampoline(0x424100, 0x424105, dsPlay_timer_vq_r);
    dsPlay_oneshot_v_t = new Trampoline(0x424FC0, 0x424FC5, dsPlay_oneshot_v_r);
    dsPlay_Dolby_timer_vq_t = new Trampoline(0x4249E0, 0x4249E5, dsPlay_Dolby_timer_vq_r);
    WriteJump((void*)0x4253B1, dsDolbySound_w);

    // Allow 2P Tails sounds in multiplayer
    WriteCall((void*)0x45C037, dsPlay_oneshot_miles); // jump
    WriteData<2>((void*)0x45C02D, 0x90ui8);
    WriteCall((void*)0x45BE01, dsPlay_oneshot_miles); // it's not always inlined!
    WriteData<2>((void*)0x45BDF4, 0x90ui8);
    WriteCall((void*)0x45BF8D, dsPlay_oneshot_miles); //hurt
    WriteCall((void*)0x45BF5D, dsPlay_oneshot_miles);
    WriteData<2>((void*)0x45BF80, 0x90ui8);
    WriteData<2>((void*)0x45BF50, 0x90ui8);
}