#include "pch.h"
#include "FunctionHook.h"
#include "UsercallFunctionHandler.h"
#include "multiplayer.h"
#include "splitscreen.h"
#include "camera.h"
#include "sound.h"

FunctionHook<void> dsLoadStageSound_h(0x424C80);

Trampoline* dsGetVolume_t = nullptr;
Trampoline* dsPlay_timer_v_t = nullptr;
Trampoline* dsPlay_timer_vq_t = nullptr;
Trampoline* dsPlay_oneshot_v_t = nullptr;
Trampoline* dsPlay_Dolby_timer_vq_t = nullptr;

int dsPlay_timer_v_r(int tone, int id, int pri, int volofs, int timer, float x, float y, float z)
{
	if (SplitScreen::IsActive() && IsCameraInSphere(x, y, z, 40000.0f))
	{
		int num = SoundQueue_GetOtherThing(tone, (EntityData1*)id); // inlined
		if (num < 0)
		{
			num = SoundQueue_GetFreeIndex(pri); // dsCheckHandle
			if (num < 0)
			{
				return -1;
			}
			sebuf[num].mode = 0x1001;
		}
		else
		{
			sebuf[num].mode &= 1;
		}

		sebuf[num].timer = timer + 1;
		sebuf[num].pri = pri;
		sebuf[num].id = id;
		sebuf[num].mode |= 0x130;
		sebuf[num].tone = tone;
		sebuf[num].angle = 0;
		sebuf[num].vol = volofs;
		sebuf[num].volmax = volofs;
		sebuf[num].pitch = 0;
		sebuf[num].pos.x = x;
		sebuf[num].pos.y = y;
		sebuf[num].pos.z = z;
		return 0;
	}
	else
	{
		return TARGET_DYNAMIC(dsPlay_timer_v)(tone, id, pri, volofs, timer, x, y, z);
	}
}

int dsPlay_timer_vq_r(int tone, int id, int pri, int volofs, int timer, float x, float y, float z, float rad)
{
	if (SplitScreen::IsActive() && IsCameraInSphere(x, y, z, rad))
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
		return 1;
	}
	else
	{
		return TARGET_DYNAMIC(dsPlay_timer_vq)(tone, id, pri, volofs, timer, x, y, z, rad);
	}
}

int dsPlay_oneshot_v_r(int tone, int id, int pri, int volofs, float x, float y, float z)
{
	if (SplitScreen::IsActive() && IsCameraInSphere(x, y, z, 40000.0f))
	{
		int num = SoundQueue_GetOtherThing(tone, (EntityData1*)id); // inlined
		if (num < 0)
		{
			num = SoundQueue_GetFreeIndex(pri); // dsCheckHandle
			if (num < 0)
			{
				return -1;
			}
			sebuf[num].mode = 0x1001;
		}
		else
		{
			sebuf[num].mode &= 1;
		}

		sebuf[num].timer = 120;
		sebuf[num].pri = -1;
		sebuf[num].id = id;
		sebuf[num].mode |= 0x110;
		sebuf[num].tone = tone;
		sebuf[num].angle = 0;
		sebuf[num].vol = volofs;
		sebuf[num].volmax = volofs;
		sebuf[num].pitch = 0;
		sebuf[num].pos.x = x;
		sebuf[num].pos.y = y;
		sebuf[num].pos.z = z;
		return 0;
	}
	else
	{
		return TARGET_DYNAMIC(dsPlay_oneshot_v)(tone, id, pri, volofs, x, y, z);
	}
}

void dsPlay_Dolby_timer_vq_r(int tone, int id, int pri, int volofs, int timer, float rad, taskwk* pTaskwk)
{
	if (SplitScreen::IsActive())
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
			sebuf[num].pos = pTaskwk->pos;
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
	int r;
	__asm
	{
		mov eax, [ii]
		call tgt
		mov r, eax
	}
	return r;
}

int __cdecl dsGetVolume_r(int ii)
{
	if (SplitScreen::IsActive())
	{
		auto se = &sebuf[ii];

		auto pnum = GetClosestPlayerNum(&se->pos);
		auto cam_pos = GetCameraPosition(pnum);
		float dist = GetDistance(&se->pos, cam_pos ? cam_pos : &playertwp[pnum]->pos);

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
		return false; // call original
	}

	for (int i = 0; i < 36; ++i)
	{
		auto se = &sebuf[i];
		auto twp = gpDolbyTask[i];

		if (se->mode & 0x4000 && twp)
		{
			auto pnum = GetClosestPlayerNum(&twp->pos);
			NJS_VECTOR v = playertwp[pnum]->pos;
			njSubVector(&v, &twp->pos);
			auto cam_ang = GetCameraAngle(pnum);

			if (cam_ang)
			{
				njPushMatrix(_nj_unit_matrix_);
				njRotateY(0, -HIWORD(GetCameraAngle(pnum)->y));
				njCalcPoint(0, &v, &v);
				njPopMatrixEx();

				v.x *= 0.1f;
				v.y *= 0.1f;
				v.z *= 0.1f;

				Set3DPositionPCM(i, v.x, v.y, v.z);
			}
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
		is_true :
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

// Allow every character sound in multiplayer
void dsLoadStageSound_r()
{
	dsLoadStageSound_h.Original();

	if (multiplayer::IsActive())
	{
		MDHeaderClose(bankhandle[3]);
		MDHeaderClose(bankhandle[6]);
		MDHeaderClose(bankhandle[8]);
		MDHeaderClose(bankhandle[9]);
		MDHeaderClose(bankhandle[10]);
		MDHeaderClose(bankhandle[11]);
		MDHeaderClose(bankhandle[12]);
		MDHeaderClose(bankhandle[13]);
		MDHeaderClose(bankhandle[14]);
		MDHeaderClose(bankhandle[15]);

		bankhandle[3] = MDHeaderOpen("SYSTEM\\SoundData\\SE\\P_SONICTAILS_BANK03.dat", 1);
		bankhandle[8] = MDHeaderOpen("SYSTEM\\SoundData\\SE\\P_KNUCKLES_BANK03.dat", 1);
		bankhandle[10] = MDHeaderOpen("SYSTEM\\SoundData\\SE\\P_AMY_BANK03.dat", 1);
		bankhandle[12] = MDHeaderOpen("SYSTEM\\SoundData\\SE\\P_BIG_BANK03.dat", 1);
		bankhandle[14] = MDHeaderOpen("SYSTEM\\SoundData\\SE\\P_E102_BANK03.dat", 1);

		if (VoiceLanguage == JAPANESE)
		{
			bankhandle[6] = MDHeaderOpen("SYSTEM\\SoundData\\SE\\V_SONICTAILS_J_BANK06.dat", 1);
			bankhandle[9] = MDHeaderOpen("SYSTEM\\SoundData\\SE\\V_KNUCKLES_J_BANK06.dat", 1);
			bankhandle[11] = MDHeaderOpen("SYSTEM\\SoundData\\SE\\V_AMY_J_BANK06.dat", 1);
			bankhandle[13] = MDHeaderOpen("SYSTEM\\SoundData\\SE\\V_BIG_J_BANK06.dat", 1);
			bankhandle[15] = MDHeaderOpen("SYSTEM\\SoundData\\SE\\V_E102_J_BANK06.dat", 1);
		}
		else
		{
			bankhandle[6] = MDHeaderOpen("SYSTEM\\SoundData\\SE\\V_SONICTAILS_E_BANK06.dat", 1);
			bankhandle[9] = MDHeaderOpen("SYSTEM\\SoundData\\SE\\V_KNUCKLES_E_BANK06.dat", 1);
			bankhandle[11] = MDHeaderOpen("SYSTEM\\SoundData\\SE\\V_AMY_E_BANK06.dat", 1);
			bankhandle[13] = MDHeaderOpen("SYSTEM\\SoundData\\SE\\V_BIG_E_BANK06.dat", 1);
			bankhandle[15] = MDHeaderOpen("SYSTEM\\SoundData\\SE\\V_E102_E_BANK06.dat", 1);
		}

		banktbl[67] = 8; // Knuckles P
		banktbl[13] = 9; // Knuckles V
		banktbl[65] = 10; // Amy P
		banktbl[11] = 11; // Amy V
		banktbl[61] = 12; // Big P
		banktbl[7] = 13; // Big V
		banktbl[63] = 14; // Gamma P
		banktbl[9] = 15; // Gamma V
	}
	else
	{
		// Remove hacks in Singleplayer
		if (banktbl[67] == 8)
		{
			banktbl[67] = 3;
			banktbl[13] = 6;
			banktbl[65] = 3;
			banktbl[11] = 6;
			banktbl[61] = 3;
			banktbl[7] = 6;
			banktbl[63] = 3;
			banktbl[9] = 6;
		}
	}
}

void InitSoundPatches()
{
	dsGetVolume_t = new Trampoline(0x4244A0, 0x4244A7, dsGetVolume_w);
	dsPlay_timer_v_t = new Trampoline(0x424000, 0x424005, dsPlay_timer_v_r);
	dsPlay_timer_vq_t = new Trampoline(0x424100, 0x424105, dsPlay_timer_vq_r);
	dsPlay_oneshot_v_t = new Trampoline(0x424FC0, 0x424FC5, dsPlay_oneshot_v_r);
	dsPlay_Dolby_timer_vq_t = new Trampoline(0x4249E0, 0x4249E5, dsPlay_Dolby_timer_vq_r);
	WriteJump((void*)0x4253B1, dsDolbySound_w);

	dsLoadStageSound_h.Hook(dsLoadStageSound_r);

	// Allow 2P Tails sounds in multiplayer
	WriteCall((void*)0x45C037, dsPlay_oneshot_miles); // jump
	WriteData<2>((void*)0x45C02D, 0x90ui8);
	WriteCall((void*)0x45BE01, dsPlay_oneshot_miles); // it's not always inlined!
	WriteData<2>((void*)0x45BDF4, 0x90ui8);
	WriteCall((void*)0x45BF8D, dsPlay_oneshot_miles); //hurt
	WriteData<2>((void*)0x45BF80, 0x90ui8);
	WriteCall((void*)0x45BF5D, dsPlay_oneshot_miles);
	WriteData<2>((void*)0x45BF50, 0x90ui8);
}
