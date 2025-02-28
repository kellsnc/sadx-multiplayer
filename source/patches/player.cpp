#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "RegisterPatch.hpp"
#include "splitscreen.h"
#include "camera.h"

FastFunctionHook<void, taskwk*, motionwk2*, playerwk*> PGetRotation_t(0x44BB60);
FastFunctionHook<void, taskwk*, motionwk2*, playerwk*> PGetAcceleration_t(0x44C270);
FastFunctionHook<void, taskwk*, motionwk2*, playerwk*, float> PGetAccelerationSnowBoard_t(0x448550);
FastFunctionHook<void, taskwk*, motionwk2*, playerwk*> PGetAccelerationForBuilding_t(0x448150);
FastUsercallHookPtr<void(*)(taskwk* twp), noret, rESI> SonicMotionCheckEdition(0x492170);
FastFunctionHook<void, taskwk*> PPlayADXAsWaiting_t(0x442360); //idle

void __cdecl PGetRotation_r(taskwk* twp, motionwk2* mwp, playerwk* pwp) // todo: rewrite
{
	if (SplitScreen::IsActive() && pwp->attr & 0x20000)
	{
		auto cam_ang = GetCameraAngle(TASKWK_PLAYERID(twp));

		if (cam_ang)
		{
			auto backup = camera_twp->ang;
			camera_twp->ang = *cam_ang;
			PGetRotation_t.Original(twp, mwp, pwp);
			camera_twp->ang = backup;
			return;
		}
	}

	PGetRotation_t.Original(twp, mwp, pwp);
}

void __cdecl PGetAcceleration_r(taskwk* twp, motionwk2* mwp, playerwk* pwp)
{
	if (SplitScreen::IsActive() && pwp->attr & 0x20000)
	{
		auto cam_ang = GetCameraAngle(TASKWK_PLAYERID(twp));

		if (cam_ang)
		{
			auto backup = camera_twp->ang;
			camera_twp->ang = *cam_ang;
			PGetAcceleration_t.Original(twp, mwp, pwp);
			camera_twp->ang = backup;
			return;
		}
	}

	PGetAcceleration_t.Original(twp, mwp, pwp);
}

void __cdecl PGetAccelerationSnowBoard_r(taskwk* twp, motionwk2* mwp, playerwk* pwp, float Max_Speed)
{
	if (SplitScreen::IsActive())
	{
		auto cam_ang = GetCameraAngle(TASKWK_PLAYERID(twp));

		if (cam_ang)
		{
			auto backup = camera_twp->ang;
			camera_twp->ang = *cam_ang;
			camera_twp->smode = GetStageNumber() == 0x802 && twp->pos.x > -5400.0f && twp->pos.y > -3900.0f ? 1 : 0;
			PGetAccelerationSnowBoard_t.Original(twp, mwp, pwp, Max_Speed);
			camera_twp->ang = backup;
			camera_twp->smode = 0;
			return;
		}
	}

	PGetAccelerationSnowBoard_t.Original(twp, mwp, pwp, Max_Speed);
}

void __cdecl PGetAccelerationForBuilding_r(taskwk* twp, motionwk2* mwp, playerwk* pwp)
{
	if (SplitScreen::IsActive())
	{
		auto cam_ang = GetCameraAngle(TASKWK_PLAYERID(twp));

		if (cam_ang)
		{
			auto backup = camera_twp->ang;
			camera_twp->ang = *cam_ang;
			PGetAccelerationForBuilding_t.Original(twp, mwp, pwp);
			camera_twp->ang = backup;
			return;
		}
	}

	PGetAccelerationForBuilding_t.Original(twp, mwp, pwp);
}

void __cdecl SonicMotionCheckEdition_r(taskwk* twp)
{
	if (SplitScreen::IsActive())
	{
		auto pnum = TASKWK_PLAYERID(twp);
		auto& per = perG[pnum];

		if (per.press & Buttons_B)
		{
			SetPlayerInitialPosition(twp);
		}

		Float lx = (per.x1 << 8);
		Float ly = (per.y1 << 8);

		if (per.on & Buttons_X)
		{
			if (lx > 3072.0f || lx < -3072.0f || ly > 3072.0f || ly < -3072.0f)
			{
				if (ly == 0.0f)
				{
					twp->pos.y = twp->pos.y - 0.0 * 5.0;
				}
				else
				{
					twp->pos.y = twp->pos.y - ly / njSqrt(ly * ly) * 5.0f;
				}
			}
		}
		else
		{
			if (lx > 3072.0 || lx < -3072.0 || ly > 3072.0f || ly < -3072.0f)
			{
				auto camera_ang = GetCameraAngle(pnum);

				if (!camera_ang)
				{
					camera_ang = &camera_twp->ang;
				}

				Float ang = -camera_ang->y - -njArcTan2(ly, lx);
				twp->pos.x = njCos(ang) * 5.0f + twp->pos.x;
				twp->pos.z = njSin(ang) * 5.0f + twp->pos.z;
			}
		}
	}
	else
	{
		SonicMotionCheckEdition.Original(twp);
	}
}

// Make idle voice works for other players
void __cdecl PPlayADXAsWaiting_r(taskwk* pTwp)
{
	if (!multiplayer::IsActive() || !pTwp || !pTwp->counter.b[0])
	{
		return PPlayADXAsWaiting_t.Original(pTwp);
	}

	const uint8_t charID = pTwp->counter.b[1];
	const Sint16 act = ssActNumber;
	auto lvlID = (ssActNumber | (ssStageNumber << 8)) >> 8;

	if (EV_CheckCansel())
		return;

	if (charID == Characters_Amy && lvlID == LevelIDs_EggCarrierInside && act == 2)
	{
		lvlID = 0;
	}

	__int16* voiceList = plADXNamePlayingAsWaiting[lvlID];
	int voiceID = 0;

	switch (charID)
	{
	case Characters_Sonic:
		voiceID = lvlID != 19 ? 390 : 396;
		break;
	case Characters_Tails:
		voiceID = 391;
		break;
	case Characters_Knuckles:
		voiceID = 392;
		break;
	case Characters_Amy:
		voiceID = 393;
		break;
	case Characters_Gamma:
		voiceID = 394;
		break;
	case Characters_Big:
		voiceID = 395;
		break;
	default:
		voiceID = ssActNumber;
		break;
	}

	if (voiceList)
	{
		if (njRandom() >= 0.5f)
		{
			int curVoice = *voiceList;
			int16_t* curLevelVoiceList = nullptr;

			if (*voiceList != -1)
			{
				while (1)
				{
					curLevelVoiceList = voiceList + 1;
					int16_t index = *curLevelVoiceList;

					if (curVoice == charID)
					{
						break;
					}

					curVoice = curLevelVoiceList[index + 1];
					voiceList = &curLevelVoiceList[index + 1];

					if (curVoice == -1)
					{
						PlayVoice(voiceID);
						return;
					}
				}

				int16_t v9 = *curLevelVoiceList;
				int16_t* index = curLevelVoiceList + 1;

				if (v9 == 1)
				{
					PlayVoice(*index);
					return;
				}

				int v13 = (njRandom() * v9);

				if (IsLevelChaoGarden())
				{
					if (v13 == v9)
					{
						v13--;
					}
				}
				else
				{
					v13 = ssAct;
				}

				int v15 = v9 - 1;
				if (v15 < v13)
				{
					v13 = v15;
				}

				voiceID = index[v13];
			}
		}
	}

	PlayVoice(voiceID);
}

void __cdecl HoldOnIcicleP_r(Uint8 pno, task* ttp)
{
	auto ptwp = playertwp[pno];

	if (!(ptwp->flag & Status_HoldObject))
	{
		SetFreeCameraMode_m(pno, FALSE);
		SetInputP(pno, PL_OP_HOLDONICICLE);
		playerpwp[pno]->htp = ttp;
	}
}

// Trick the game to draw shadows on other screens
Bool ChkPause_Shadow_Hack()
{
	return ChkPause() || SplitScreen::numScreen != 0;
}

// The game sometimes wrongly check for player 1 while the rest of the code doesn't
// We force these cases to check for any player
Bool CheckPlayerRideOnMobileLandObjectP_P1_Hack(unsigned __int8 pno, task* ttp)
{
	if (multiplayer::IsActive())
	{
		return IsPlayerOnDyncol(ttp);
	}
	else
	{
		return CheckPlayerRideOnMobileLandObjectP(pno, ttp);
	}
}

void patch_player_init()
{
	PGetRotation_t.Hook(PGetRotation_r);
	PGetAcceleration_t.Hook(PGetAcceleration_r);
	PGetAccelerationSnowBoard_t.Hook(PGetAccelerationSnowBoard_r);
	PGetAccelerationForBuilding_t.Hook(PGetAccelerationForBuilding_r);
	SonicMotionCheckEdition.Hook(SonicMotionCheckEdition_r);
	PPlayADXAsWaiting_t.Hook(PPlayADXAsWaiting_r); //patch idle voice multiplayer
	WriteJump(HoldOnIcicleP, HoldOnIcicleP_r); // Disable free camera for the proper player on icicles

	// Game draws shadow in logic sub but also in display sub *if* game is paused.
	// To keep compatibility with mods (like SA2 Sonic), I just force the display for the other screens.
	// Better alternative is to skip display in logic and always draw in display, but I chose max compatibility.
	WriteCall((void*)0x494B57, ChkPause_Shadow_Hack); // Sonic
	WriteCall((void*)0x461420, ChkPause_Shadow_Hack); // Tails
	WriteCall((void*)0x472674, ChkPause_Shadow_Hack); // Knuckles
	WriteCall((void*)0x4875F9, ChkPause_Shadow_Hack); // Amy
	WriteCall((void*)0x48BA5A, ChkPause_Shadow_Hack); // Big
	WriteCall((void*)0x480702, ChkPause_Shadow_Hack); // Gamma

	// Patch CheckPlayerRideOnMobileLandObjectP occurences that don't need full rewrites
	WriteCall((void*)0x4CB36C, CheckPlayerRideOnMobileLandObjectP_P1_Hack); // Switch
	WriteCall((void*)0x52130E, CheckPlayerRideOnMobileLandObjectP_P1_Hack); // OStation
	WriteCall((void*)0x522F7E, CheckPlayerRideOnMobileLandObjectP_P1_Hack); // OSidelift
	WriteCall((void*)0x523185, CheckPlayerRideOnMobileLandObjectP_P1_Hack); // OSidelift
	WriteCall((void*)0x526AAE, CheckPlayerRideOnMobileLandObjectP_P1_Hack); // OBlift
	WriteCall((void*)0x52A5CE, CheckPlayerRideOnMobileLandObjectP_P1_Hack); // OStation
	WriteCall((void*)0x59DC87, CheckPlayerRideOnMobileLandObjectP_P1_Hack); // OUkijima
	WriteCall((void*)0x59DCA0, CheckPlayerRideOnMobileLandObjectP_P1_Hack); // OUkijima
	WriteCall((void*)0x5AC110, CheckPlayerRideOnMobileLandObjectP_P1_Hack); // Cargo
	WriteCall((void*)0x5AC153, CheckPlayerRideOnMobileLandObjectP_P1_Hack); // Cargo
	WriteCall((void*)0x5E6B26, CheckPlayerRideOnMobileLandObjectP_P1_Hack); // Aokiswitch
	WriteCall((void*)0x5FA25C, CheckPlayerRideOnMobileLandObjectP_P1_Hack); // Edge
	WriteCall((void*)0x5E6B26, CheckPlayerRideOnMobileLandObjectP_P1_Hack); // Connect
	WriteCall((void*)0x5E6B26, CheckPlayerRideOnMobileLandObjectP_P1_Hack); // Talap
	WriteCall((void*)0x601FB0, CheckPlayerRideOnMobileLandObjectP_P1_Hack); // ExplosionRock
	WriteCall((void*)0x63D865, CheckPlayerRideOnMobileLandObjectP_P1_Hack);
	WriteCall((void*)0x63D90D, CheckPlayerRideOnMobileLandObjectP_P1_Hack);
	WriteCall((void*)0x63D940, CheckPlayerRideOnMobileLandObjectP_P1_Hack);
	WriteCall((void*)0x63DFB6, CheckPlayerRideOnMobileLandObjectP_P1_Hack);
	WriteCall((void*)0x63DFED, CheckPlayerRideOnMobileLandObjectP_P1_Hack);
}

RegisterPatch patch_player(patch_player_init);