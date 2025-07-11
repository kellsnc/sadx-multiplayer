#include "pch.h"
#include "SADXModLoader.h"
#include "config.h"
#include "hud_indicator.h"
#include "milesrace.h"
#include "result.h"
#include "splitscreen.h"
#include "teleport.h"
#include "testspawn.h"
#include "multiplayer.h"

#ifdef MULTI_NETPLAY
#include "netplay.h"
#include "packet.h"
#include "timer.h"
#endif

// Where multiplayer happens.
// - Load all requested players ingame
// - Add ring/lives/score variables for all players
// - Patch start positions
// - Sync players in netplay

DataPointer(GM_START_POSANG*, paSonicIP_Ptr, 0x41491E);
DataPointer(GM_START_POSANG*, paMilesIP_Ptr, 0x414925);
DataPointer(GM_START_POSANG*, paKnucklesIP_Ptr, 0x41492C);
DataPointer(GM_START_POSANG*, paAmyIP_Ptr, 0x41493A);
DataPointer(GM_START_POSANG*, paE102IP_Ptr, 0x414941);
DataPointer(GM_START_POSANG*, paBigIP_Ptr, 0x414933);

DataPointer(ADVPOS**, vInitialPositionSS_Ptr, 0x62F6EE);
DataPointer(ADVPOS**, vInitialPositionEC_AB_Ptr, 0x52D853);
DataPointer(ADVPOS**, vInitialPositionEC_C_Ptr, 0x52D861);
DataPointer(ADVPOS**, vInitialPositionMR_Ptr, 0x5307AE);
DataPointer(ADVPOS**, vInitialPositionPast_Ptr, 0x54219E);


#ifdef MULTI_NETPLAY
static Timer update_timer(std::chrono::steady_clock::duration(std::chrono::milliseconds(100)));
static constexpr uint16_t FLAG_MASK = Status_Ball | Status_Attack | Status_LightDash;
#endif

VariableHook<int16_t, 0x3B0F0E4> ssNumRing_m;
VariableHook<int8_t, 0x3B0EF34> scNumPlayer_m;
VariableHook<int32_t, 0x3B0F104> EnemyScore_m;

FastFunctionHook<void> SetPlayer_h(SetPlayer);
FastFunctionHook<void, taskwk*> SetPlayerInitialPosition_h(0x414810);

TaskFuncPtr charfuncs[] = {
	SonicTheHedgehog,
	(TaskFuncPtr)Eggman_Main,
	MilesTalesPrower,
	KnucklesTheEchidna,
	(TaskFuncPtr)Tikal_Main,
	AmyRose,
	E102,
	BigTheCat
};

namespace multiplayer
{
	bool enabled = false;
	unsigned int pcount = 0;
	mode gMode = mode::coop;
	bool online = false;
	static int characters[PLAYER_MAX] = { -1, -1, -1, -1 };

	unsigned int GetPlayerCount()
	{
		return pcount;
	}

	bool Enable(int player_count, mode md)
	{
		if (!enabled && player_count > 1 && player_count <= PLAYER_MAX)
		{
			gMode = md;
			enabled = true;
			pcount = player_count;
			splitscreen::Enable();
			return true;
		}
		return false;
	}

	bool Disable()
	{
		if (enabled)
		{
			enabled = false;
			pcount = 0;
			multiplayer::ResetCharacters();
			splitscreen::Disable();
			return true;
		}
		return false;
	}

	bool IsEnabled()
	{
		return enabled;
	}

	bool IsActive()
	{
		return IsEnabled() && (GameMode != GameModes_Menu) && pcount > 1;
	}

	bool IsBattleMode()
	{
		return IsActive() && gMode == mode::battle;
	}

	bool IsCoopMode()
	{
		return IsActive() && gMode == mode::coop;
	}

	bool IsFightMode()
	{
		return IsActive() && gMode == mode::fight;
	}

	bool IsAdventureMode()
	{
		return IsActive() && ulGlobalMode == MD_ADVENTURE;
	}

	bool IsOnline()
	{
		return online;
	}

	void SetCharacter(int pnum, int character)
	{
		characters[pnum] = character;
	}

	int GetCharacter(int pnum)
	{
		if (pnum == 0 && characters[0] == -1)
		{
			return CurrentCharacter;
		}

		return characters[pnum];
	}

	void ResetCharacters()
	{
		for (auto& character : characters)
		{
			character = -1;
		}
	}
}

task* LoadPlayerTask(int pnum, int character)
{
	task* tp;

	if (ssStageNumber == LevelIDs_SkyChase1 || ssStageNumber == LevelIDs_SkyChase2)
	{
		tp = CreateElementalTask(LoadObj_UnknownA | LoadObj_Data1 | LoadObj_Data2, 1, (TaskFuncPtr)Tornado_Main);
	}
	else
	{
		tp = CreateElementalTask((LoadObj_UnknownA | LoadObj_Data1 | LoadObj_Data2), LEV_1, charfuncs[character]);
	}
	TASKWK_CHARID(tp->twp) = character;
	TASKWK_PLAYERID(tp->twp) = pnum;
	playertwp[pnum] = tp->twp;
	playermwp[pnum] = (motionwk2*)tp->mwp;
	return tp;
}

void SetOtherPlayers()
{
	if (!multiplayer::IsActive())
	{
		return;
	}

	int count = 0;
	for (int i = 1; i < PLAYER_MAX; i++)
	{
		int character = multiplayer::GetCharacter(i);

		if (character >= 0)
		{
			if (++count >= multiplayer::GetPlayerCount())
				break;

			if (!playertwp[i])
			{
				if (LoadPlayerTask(i, character))
				{
					TeleportPlayerArea(i, &playertwp[0]->pos, 5.0f);
					playertwp[i]->ang = playertwp[0]->ang;
				}
			}
		}
	}
}

// We load all players instead of just P1
void SetPlayer_r()
{
	if (multiplayer::IsActive())
	{
		LoadPlayerTask(0, multiplayer::GetCharacter(0));
		SetOtherPlayers();

		// Load game mechanics:
		switch (CurrentCharacter)
		{
		case Characters_Tails:
			if (multiplayer::IsCoopMode())
			{
				Set_NPC_Sonic_m(NPC_PNUM); // load opponent into slot 7
			}
			break;
		case Characters_Knuckles:
			if (!EV_CheckCansel() && (ulGlobalMode == 4 || ulGlobalMode == 10 || ulGlobalMode == 9))
			{
				CreateElementalTask(2u, 6, Knuckles_KakeraGame);
			}
			break;
		case Characters_Big:
			if (CharacterSelectEnabled)
				CreateElementalTask(2u, 6, BigDisplayStatus);
			break;
		}

		CreateIndicatorP();
		SetWinnerMulti(-1);
		TeleportPlayersToStart();
	}
	else
	{
		return SetPlayer_h.Original();
	}
}

void GetPlayerInitialPositionM(NJS_POINT3* pos, Angle3* ang)
{
	if (gTestSpawnStartPos)
	{
		Angle3 angy = { 0, gTestSpawnStartPos->angy, 0 };
		updateContinueData(&gTestSpawnStartPos->pos, &angy);
		delete gTestSpawnStartPos;
		gTestSpawnStartPos = nullptr;
	}

	if (CheckContinueData())
	{
		SetTime2(RestartLevel.Minute, RestartLevel.Second, RestartLevel.Frame);
		*pos = continue_data.pos;
		*ang = continue_data.ang;
		return;
	}

	if (FieldStartPos)
	{
		*pos = FieldStartPos->Position;
		*ang = { 0, FieldStartPos->YRot, 0 };
		FieldStartPos = nullptr;
		return;
	}

	if (IsInAdventureField())
	{
		ADVPOS* adpos;

		// Adv Field:
		switch (ssStageNumber)
		{
		default:
		case LevelIDs_StationSquare:
		case 27:
		case 28:
			adpos = &vInitialPositionSS_Ptr[ssActNumber][GetLevelEntranceID()];
			break;
		case LevelIDs_EggCarrierOutside:
			adpos = &vInitialPositionEC_AB_Ptr[ssActNumber][GetLevelEntranceID()];
			break;
		case LevelIDs_EggCarrierInside:
			adpos = &vInitialPositionEC_C_Ptr[ssActNumber][GetLevelEntranceID()];
			break;
		case LevelIDs_MysticRuins:
			adpos = &vInitialPositionMR_Ptr[ssActNumber][GetLevelEntranceID()];
			break;
		case LevelIDs_Past:
			adpos = &vInitialPositionPast_Ptr[ssActNumber][GetLevelEntranceID()];
			break;
		}

		*pos = adpos->pos;
		*ang = { 0, adpos->angy, 0 };
		return;
	}

	GM_START_POSANG* stpos;

	switch (CurrentCharacter)
	{
	default:
	case Characters_Sonic:
		stpos = paSonicIP_Ptr;
		break;
	case Characters_Tails:
		stpos = paMilesIP_Ptr;
		break;
	case Characters_Knuckles:
		stpos = paKnucklesIP_Ptr;
		break;
	case Characters_Amy:
		stpos = paAmyIP_Ptr;
		break;
	case Characters_Gamma:
		stpos = paE102IP_Ptr;
		break;
	case Characters_Big:
		stpos = paBigIP_Ptr;
		break;
	}

	while (stpos->stage != STAGE_NUMBER)
	{
		if (stpos->stage == ssStageNumber && stpos->act == ssActNumber)
		{
			*pos = continue_data.pos = stpos->p;
			*ang = continue_data.ang = { 0, stpos->angy, 0 };
			updateContinueData(pos, ang);
			return;
		}
		++stpos;
	}

	*pos = { 0.0f, 0.0f, 0.0f };
	*ang = { 0, 0, 0 };
}

void SetPlayerInitialPosition_r(taskwk* twp)
{
	if (multiplayer::IsActive())
	{
		TeleportPlayerToStart(TASKWK_PLAYERID(twp));
	}
	else
	{
		SetPlayerInitialPosition_h.Original(twp);
	}
}

void ResetEnemyScoreM()
{
	Score = 0;
	EnemyScore_m.clear();
	ComboScore = 0;
	ComboTimer = 0;
	*(int*)0x3B0F138 = 0;
}

int GetEnemyScoreM(int pNum)
{
	return EnemyScore_m[pNum];
}

void AddEnemyScoreM(int pNum, int add)
{
	EnemyScore_m[pNum] += add;
}

void SetEnemyScoreM(int pNum, int Number)
{
	EnemyScore_m[pNum] = Number;
}

void ResetNumPlayerM()
{
	std::fill(scNumPlayer_m.begin(), scNumPlayer_m.end(), *(Sint8*)0x425AF6);
}

int GetNumPlayerM(int pNum)
{
	return (multiplayer::IsCoopMode()) ? scNumPlayer_m[0] : scNumPlayer_m[pNum];
}

void AddNumPlayerM(int pNum, int Number)
{
	if (multiplayer::IsActive())
	{
		if (Number > 0)
		{
			PlaySound(743, 0, 0, 0);
		}

		if (pNum > 0 && multiplayer::IsCoopMode())
			pNum = 0;

		auto& counter = scNumPlayer_m[pNum];

		counter += Number;

		if ((counter < 0 && Number > 0) || Number >= CHAR_MAX)
		{
			counter = CHAR_MAX;
		}

		if (GetLevelType() == 1)
		{
			LoadObject(LoadObj_UnknownB, 6, sub_425B30);
		}
	}
	else
	{
		AddNumPlayer(Number);
	}
}

void SetNumPlayerM(int pNum, int Number)
{
	scNumPlayer_m[pNum] = (multiplayer::IsCoopMode() && pNum > 0) ? scNumPlayer_m[0] : Number;
}

int GetNumRingM(int pNum)
{
	return ssNumRing_m[pNum];
}

int GetTotalRingsM()
{
	int total = 0;
	if (multiplayer::IsActive())
	{
		for (uint8_t i = 0; i < multiplayer::GetPlayerCount(); i++)
		{
			total += GetNumRingM(i);
		}

	}

	return total;
}

void AddNumRingM(int pNum, int add)
{
	if (multiplayer::IsActive())
	{
		int origc, newc = 0;

		auto& counter = ssNumRing_m[pNum];

		origc = counter / 100;
		counter += add;
		newc = counter / 100;

		if (origc < newc)
		{
			AddNumPlayerM(pNum, newc - origc);
		}

		if (GetLevelType() == 1)
		{
			LoadObject(LoadObj_UnknownB, 6, sub_425BB0);
		}
	}
	else
	{
		AddNumRing(add);
	}
}

void SetNumRingM(int pNum, int Number)
{
	ssNumRing_m[pNum] = Number;
}

void ResetNumRingM()
{
	ssNumRing_m.clear();
}

void ResetNumRingP(int pNum)
{
	ssNumRing_m[pNum] = 0;
}

//for casino
static void GetRingNowM(task* tp)
{
	taskwk* twp = tp->twp;
	const Uint8 pnum = twp->counter.b[0];

	if (twp->wtimer == 0x1234) //magicnumber 
	{
		if (GetNumRingM(pnum))
		{
			if (twp->btimer--)
			{
				AddNumRingM(pnum, -1);
				return;
			}
		}
		FreeTask(tp);
		return;
	}

	if (!twp->wtimer--)
	{
		if (!twp->btimer--)
		{
			FreeTask(tp);
			return;
		}
		twp->wtimer = 6;
		AddNumRingM(pnum, 1);
		dsPlay_oneshot(SE_RING, 0, 0, 0);
	}
}

/// <summary>
/// Increment ring counter.
/// </summary>
/// <param name="RingCount"></param>
void SetGetRingM(Uint8 RingCount, const Uint8 pnum)
{
	taskwk* twp = CreateElementalTask(IM_TASKWK, LEV_3, GetRingNowM)->twp;
	twp->wtimer = 5;
	twp->btimer = RingCount;
	twp->counter.b[0] = pnum;
}

/// <summary>
/// Decrement ring counter.
/// </summary>
/// <param name="RingCount"></param>
void SetDropRingM(Uint8 RingCount, const Uint8 pnum)
{
	taskwk* twp = CreateElementalTask(IM_TASKWK, LEV_3, GetRingNowM)->twp;
	twp->wtimer = 0x1234;
	twp->btimer = RingCount;
	twp->counter.b[0] = pnum;
}

// Reset all rings instead of just P1
void InitScore_r()
{
	ResetNumRingM();
	slJudge = 0;
}

#ifdef MULTI_NETPLAY
static bool PlayerListener(Packet& packet, Netplay::PACKET_TYPE type, Netplay::PNUM pnum)
{
	auto ptwp = playertwp[pnum];
	auto pmwp = playermwp[pnum];
	auto ppwp = playerpwp[pnum];

	if (!ptwp || !ppwp || !pmwp)
	{
		return false;
	}

	switch (type)
	{
	case Netplay::PACKET_PLAYER_LOCATION:
		packet >> ptwp->pos >> ptwp->ang >> ppwp->spd >> pmwp->spd;
		return true;
	case Netplay::PACKET_PLAYER_MODE:
		packet >> ptwp->mode;
		return true;
	case Netplay::PACKET_PLAYER_SMODE:
		packet >> ptwp->smode;
		ptwp->flag |= Status_DoNextAction;
		return true;
	case Netplay::PACKET_PLAYER_FLAG:
	{
		short flag;
		packet >> flag;
		ptwp->flag = (ptwp->flag & ~FLAG_MASK) | (flag & FLAG_MASK);
		return true;
	}
	case Netplay::PACKET_PLAYER_ANIM:
		packet >> ppwp->mj.mtnmode >> (ppwp->mj.mtnmode == 2 ? ppwp->mj.action : ppwp->mj.reqaction) >> ppwp->mj.nframe;
		return true;
	case Netplay::PACKET_PLAYER_RINGS:
		packet >> ssNumRing_m[pnum];
		return true;
	case Netplay::PACKET_PLAYER_LIVES:
		packet >> scNumPlayer_m[pnum];
		return true;
	case Netplay::PACKET_PLAYER_SCORE:
		packet >> EnemyScore_m[pnum];
		return true;
	default:
		return false;
	}
}

static bool PlayerSender(Packet& packet, Netplay::PACKET_TYPE type, Netplay::PNUM pnum)
{
	auto ptwp = playertwp[pnum];
	auto ppwp = playerpwp[pnum];
	auto pmwp = playermwp[pnum];

	switch (type)
	{
	case Netplay::PACKET_PLAYER_LOCATION:
		packet << ptwp->pos << ptwp->ang << ppwp->spd << pmwp->spd;
		return true;
	case Netplay::PACKET_PLAYER_MODE:
		packet << ptwp->mode;
		return true;
	case Netplay::PACKET_PLAYER_SMODE:
		packet << ptwp->smode;
		return true;
	case Netplay::PACKET_PLAYER_FLAG:
		packet << (short)(ptwp->flag & FLAG_MASK);
		return true;
	case Netplay::PACKET_PLAYER_ANIM:
		packet << ppwp->mj.mtnmode << (ppwp->mj.mtnmode == 2 ? ppwp->mj.action : ppwp->mj.reqaction) << ppwp->mj.nframe;
		return true;
	case Netplay::PACKET_PLAYER_RINGS:
		packet << ssNumRing_m[pnum];
		return true;
	case Netplay::PACKET_PLAYER_LIVES:
		packet << scNumPlayer_m[pnum];
		return true;
	case Netplay::PACKET_PLAYER_SCORE:
		packet << EnemyScore_m[pnum];
		return true;
	default:
		return false;
	}
}
#endif

void ExecMultiplayer()
{
#ifdef _DEBUG
	if ((perG[1].press & Buttons_L) && (perG[1].press & Buttons_R))
	{
		if (ChkGameMode() && playertwp[1] && playertwp[0])
		{
			TeleportPlayer(1, &playertwp[0]->pos);
		}
	}
#endif

	bool coop = multiplayer::IsCoopMode();

	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		auto& cheats = config.cheats;

		if (cheats.mInfiniteLives[i])
		{
			scNumPlayer_m[i] = 99;
		}

		if (cheats.mInfiniteRings[i])
		{
			ssNumRing_m[i] = 999;
		}

		// Todo: rework this...
		auto ptwp = playertwp[i];
		if (ptwp && ptwp->cwp)
		{
			if (CharacterBossActive)
			{
				for (int col = 0; col < ptwp->cwp->nbInfo; col++)
				{
					ptwp->cwp->info[col].damage |= 0x20; // Restore damage
					ptwp->cwp->info[col].push |= 0x1; // Restore push
				}
			}
			else if (coop && !EV_MainThread_ptr)
			{
				for (int col = 0; col < ptwp->cwp->nbInfo; col++)
				{
					ptwp->cwp->info[col].damage &= ~0x20; // Remove damage
					ptwp->cwp->info[col].push &= ~0x1; // Remove push
				}
			}
		}
	}

#ifdef MULTI_NETPLAY
	if (netplay.IsConnected())
	{
		auto pnum = netplay.GetPlayerNum();
		auto ptwp = playertwp[pnum];
		auto ppwp = playerpwp[pnum];

		if (ptwp && ppwp)
		{
			if (update_timer.Finished())
			{
				netplay.Send(Netplay::PACKET_PLAYER_LOCATION, PlayerSender);
				netplay.Send(Netplay::PACKET_PLAYER_ANIM, PlayerSender);
				netplay.Send(Netplay::PACKET_PLAYER_FLAG, PlayerSender);
			}

			static char last_action = 0;
			if (last_action != ptwp->mode)
			{
				netplay.Send(Netplay::PACKET_PLAYER_MODE, PlayerSender);
				last_action = ptwp->mode;
			}

			if (ptwp->flag & Status_DoNextAction)
			{
				netplay.Send(Netplay::PACKET_PLAYER_SMODE, PlayerSender);
			}

			static short last_mtnmode = 0;
			static short last_mtnaction = 0;
			if (last_mtnmode != ppwp->mj.mtnmode || last_mtnaction != (last_mtnmode == 2 ? ppwp->mj.action : ppwp->mj.reqaction))
			{
				netplay.Send(Netplay::PACKET_PLAYER_ANIM, PlayerSender);
				last_mtnaction = (last_mtnmode == 2 ? ppwp->mj.action : ppwp->mj.reqaction);
				last_mtnmode = ppwp->mj.mtnmode;
			}

			static int16_t old_rings = 0;
			if (old_rings != ssNumRing_m[pnum])
			{
				netplay.Send(Netplay::PACKET_PLAYER_RINGS, PlayerSender);
				old_rings = ssNumRing_m[pnum];
			}

			static int8_t old_lives = 0;
			if (old_lives != scNumPlayer_m[pnum])
			{
				netplay.Send(Netplay::PACKET_PLAYER_LIVES, PlayerSender);
				old_lives = scNumPlayer_m[pnum];
			}

			static int32_t old_score = 0;
			if (old_score != EnemyScore_m[pnum])
			{
				netplay.Send(Netplay::PACKET_PLAYER_SCORE, PlayerSender);
				old_score = EnemyScore_m[pnum];
			}
		}
	}
#endif
}

void InitMultiplayer()
{
	SetPlayer_h.Hook(SetPlayer_r);
	SetPlayerInitialPosition_h.Hook(SetPlayerInitialPosition_r);

	WriteJump(ResetNumPlayer, ResetNumPlayerM);
	WriteJump(ResetNumRing, ResetNumRingM);
	WriteJump(InitActionScore, ResetEnemyScoreM);
	WriteJump(InitScore, InitScore_r);

#ifdef MULTI_NETPLAY
	netplay.RegisterListener(Netplay::PACKET_PLAYER_LOCATION, PlayerListener);
	netplay.RegisterListener(Netplay::PACKET_PLAYER_MODE, PlayerListener);
	netplay.RegisterListener(Netplay::PACKET_PLAYER_SMODE, PlayerListener);
	netplay.RegisterListener(Netplay::PACKET_PLAYER_FLAG, PlayerListener);
	netplay.RegisterListener(Netplay::PACKET_PLAYER_ANIM, PlayerListener);

	netplay.RegisterListener(Netplay::PACKET_PLAYER_RINGS, PlayerListener);
	netplay.RegisterListener(Netplay::PACKET_PLAYER_LIVES, PlayerListener);
	netplay.RegisterListener(Netplay::PACKET_PLAYER_SCORE, PlayerListener);
#endif
}