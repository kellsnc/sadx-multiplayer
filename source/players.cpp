#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "VariableHook.hpp"
#include "netplay.h"
#include "timer.h"
#include "multiplayer.h"
#include "result.h"
#include "milesrace.h"
#include "hud_indicator.h"
#include "config.h"
#include "menu_multi.h"
#include "teleport.h"
#include "players.h"

/*

Multiplayer manager
- Extends player variables
- Load available players
- Patch start positions
- Sync players in netplay

*/

#ifdef MULTI_NETPLAY
static Timer update_timer(std::chrono::steady_clock::duration(std::chrono::milliseconds(100)));
#endif

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

FastFunctionHook<void, taskwk*> SetPlayerInitialPosition_h(0x414810);
FastFunctionHook<void, char> DamegeRingScatter_h(DamegeRingScatter);
FastFunctionHook<void> SetPlayer_h(SetPlayer);
FastFunctionHook<Bool, taskwk*> isInDeathZone_h((intptr_t)IsInDeathZone_);

VariableHook<int16_t, 0x3B0F0E4> ssNumRing_m;
VariableHook<int8_t, 0x3B0EF34> scNumPlayer_m;
VariableHook<int32_t, 0x3B0F104> EnemyScore_m;

ADVPOS* gTestSpawnStartPos = nullptr;

static bool isCharSel = false;
static int characters[PLAYER_MAX] = { -1, -1, -1, -1 };

static constexpr uint16_t FLAG_MASK = Status_Ball | Status_Attack | Status_LightDash;
FastFunctionHook<int> GetRaceWinnerPlayer_h(GetRaceWinnerPlayer);

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

void DamegeRingScatter_r(char pno)
{
	if (multiplayer::IsActive())
	{
		auto rings = GetNumRingM(pno);

		if (rings > 0)
		{
			ResetNumRingP(pno);

			for (int i = 0; i < min(20, rings); ++i)
			{
				auto tp = CreateElementalTask(LoadObj_UnknownB | LoadObj_Data1, 2, (TaskFuncPtr)0x44FD10);
				tp->twp->pos = playertwp[pno]->pos;
				tp->twp->ang.y = NJM_DEG_ANG(((double)(i * 350.0) / (double)rings) + (njRandom() * 360.0));
			}

			dsPlay_oneshot(0, 0, 0, 0);
		}
		else
		{
			KillHimP(pno);

			if (TASKWK_CHARID(playertwp[pno]) == Characters_Gamma)
			{
				dsPlay_oneshot(1431, 0, 0, 0);
			}
			else
			{
				dsPlay_oneshot(23, 0, 0, 0);
			}
		}
	}
	else
	{
		return DamegeRingScatter_h.Original(pno);
	}
}

// Remove ability to be hurt by players
void RemovePlayersDamage(taskwk* twp)
{
	if (twp && twp->cwp)
	{
		for (int i = 0; i < twp->cwp->nbInfo; i++)
		{
			twp->cwp->info[i].damage &= ~0x20u;
		}
	}
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

void UpdatePlayersInfo()
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

void ResetCharactersArray()
{
	for (int i = 0; i < PLAYER_MAX; i++)
	{
		characters[i] = -1;
	}
}

void SetCurrentCharacter(int pnum, Characters character)
{
	characters[pnum] = character;
}

int GetCurrentCharacter(int pnum)
{
	return characters[pnum];
}

task* LoadPlayerTask(int pnum, int character)
{
	task* tp = CreateElementalTask((LoadObj_UnknownA | LoadObj_Data1 | LoadObj_Data2), LEV_1, charfuncs[character]);
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
		int playernum = characters[i];

		if (playernum >= 0)
		{
			if (++count >= multiplayer::GetPlayerCount())
				break;

			if (!playertwp[i])
			{
				if (LoadPlayerTask(i, playernum))
				{
					TeleportPlayerArea(i, &playertwp[0]->pos, 5.0f);
					playertwp[i]->ang = playertwp[0]->ang;
				}
			}
		}
	}
}

void SetPlayer_r()
{
	if (multiplayer::IsActive())
	{
		LoadPlayerTask(0, characters[0] < 0 ? CurrentCharacter : characters[0]);
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
			if (isCharSel)
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

void InitScore_r()
{
	ResetNumRingM();
	slJudge = 0;
}

int GetRaceWinnerPlayer_r()
{
	if (multiplayer::IsCoopMode())
	{
		for (int i = 1; i < PLAYER_MAX; i++)
		{
			if (playertwp[i] && (TASKWK_CHARID(playertwp[i]) == Characters_Tails))
				return 1;
		}
	}

	return GetRaceWinnerPlayer_h.Original();
}

Bool isInDeathZone_r(taskwk* a1)
{
	if (multiplayer::IsCoopMode() && CurrentCharacter != Characters_Knuckles)
	{
		return 0;
	}

	return isInDeathZone_h.Original(a1);
}

bool DeleteJiggle(task* tp)
{
	if (multiplayer::IsActive() && !EV_MainThread_ptr)
	{
		FreeTask(tp);
		return true;
	}

	return false;
}

void InitPlayerPatches()
{
	isCharSel = GetModuleHandle(L"SADXCharSel") != nullptr;

	SetPlayerInitialPosition_h.Hook(SetPlayerInitialPosition_r);
	DamegeRingScatter_h.Hook(DamegeRingScatter_r);
	SetPlayer_h.Hook(SetPlayer_r);
	isInDeathZone_h.Hook(isInDeathZone_r);

	WriteJump(ResetNumPlayer, ResetNumPlayerM);
	WriteJump(ResetNumRing, ResetNumRingM);
	WriteJump(InitActionScore, ResetEnemyScoreM);
	WriteJump(InitScore, InitScore_r);
	GetRaceWinnerPlayer_h.Hook(GetRaceWinnerPlayer_r); //fix wrong victory pose for Tails.

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
