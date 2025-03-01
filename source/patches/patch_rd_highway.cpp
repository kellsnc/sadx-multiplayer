#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "multiplayer.h"
#include "splitscreen.h"
#include "result.h"

// Mess to make Speed Highway playable (especially the second act)

DataPointer(NJS_OBJECT, object_s1_nbg1_nbg1, 0x26A0EC0);
DataPointer(NJS_OBJECT, object_s2_yakei_yakei, 0x26A48E0);
VoidFunc(HighwayMaskBlock, 0x60FEE0); // checkCamera
VoidFunc(RdHighwayManageLandMask, 0x60FEE0); // ?

FastUsercallHookPtr<TaskFuncPtr, noret, rEAX> RdHighwayCheckArriveAtTheBuilding_h(0x610050);
FastFunctionHookPtr<TaskFuncPtr> subRd_Highway_h(0x6104C0);
FastFunctionHookPtr<TaskFuncPtr> dispBgHighway_h(0x610570);

static void RdHighwayAct2Multi(taskwk* twp)
{
	for (int i = 0; i < PLAYER_MAX; ++i)
	{
		auto ptwp = playertwp[i];

		if (ptwp)
		{
			if (ptwp->pos.y < -18800.0f)
			{
				if (ptwp->smode != 24)
				{
					SetInputP(i, PL_OP_LETITGO);
					SetVelocityP(i, 1.4f, -2.0f, playermwp[i]->spd.z);

					if (!i) twp->mode = 5;
				}

				if (ptwp->pos.x >= 230.0f)
				{
					if (!i) twp->mode = 6;

					if (ptwp->pos.x >= 280.0f)
					{
						twp->mode = 7;
						//PadReadOnP(-1);
						ChangeActM(1);
						dsPlay_iloop(961, -1, 8, 0);
						auto tp = CreateElementalTask(LoadObj_Data1, LEV_0, (TaskFuncPtr)0x61CA90);
						tp->twp->mode = MusicIDs_SpeedHighwayAtDawn;
						tp->twp->wtimer = 10;
						return;
					}
				}
			}
			else if (ptwp->pos.y < -150.0f)
			{
				if (ptwp->smode != 43)
				{
					if (CurrentSong != MusicIDs_SpeedHighwayGoinDown)
					{
						auto tp = CreateElementalTask(LoadObj_Data1, LEV_0, (TaskFuncPtr)0x61CA90);
						tp->twp->mode = MusicIDs_SpeedHighwayGoinDown;
						tp->twp->wtimer = 5;
					}

					SetInputP(i, PL_OP_BUILDING);
					//PadReadOnP(i);

					if (!i) twp->mode = 4;
				}
			}
			else if (ptwp->pos.x > 0.0f)
			{
				//PadReadOffP(i);
				if (!i) twp->mode = 3;
			}
		}
	}
}

static void RdHighwayCheckArriveAtTheBuilding_m(taskwk* twp)
{
	if (IsPlayerInSphere(4002.0f, -1500.0f, 4750.0f, 60.0f))
	{
		twp->mode = 2;
		ChangeActM(1);
		dsPlay_iloop(963, -1, 8, 0);
	}
}

// Custom display to manage landtable display flags for every player
static void __cdecl RdHighwayDisp(task* tp)
{
	if (multiplayer::IsActive())
	{
		RdHighwayManageLandMask();
	}
}

static void __cdecl subRd_Highway_r(task* tp)
{
	auto twp = tp->twp;

	if (multiplayer::IsActive())
	{
		switch (twp->mode)
		{
		case 0:
			RdHighwayInit(tp);
			tp->disp = RdHighwayDisp;
		case 1:

			// End point is hardcoded in Tails' Speed Highway so we must check it here
			if (CurrentCharacter == Characters_Tails)
			{
				if (twp->smode == 0)
				{
					for (int i = 0; i < PLAYER_MAX; ++i)
					{
						auto ptwp = playertwp[i];

						if (ptwp)
						{
							VecTemp0.x = 10335.0f - ptwp->pos.x;
							VecTemp0.y = -1974.0f - ptwp->pos.y;
							VecTemp0.z = 10177.0f - ptwp->pos.z;

							if (njScalor2(&VecTemp0) < 22500.0f)
							{
								SetWinnerMulti(i);
								SetFinishAction();
								twp->smode = 1;
								break;
							}
						}
					}
				}
			}
			else
			{
				RdHighwayCheckArriveAtTheBuilding_m(twp);
			}

			break;
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
			RdHighwayAct2Multi(twp);
			break;
		case 7:
			RdHighwayManageLandMask();
			break;
		}
	}
	else
	{
		subRd_Highway_h.Original(tp);
	}
}

static void __cdecl RdHighwayCheckArriveAtTheBuilding_r(task* tp)
{
	if (multiplayer::IsActive())
	{
		RdHighwayCheckArriveAtTheBuilding_m(tp->twp);
	}
	else
	{
		RdHighwayCheckArriveAtTheBuilding_h.Original(tp);
	}
}

// Fix display masks
static void __cdecl dispBgHighway_r(task* tp)
{
	if (tp->twp->mode == 4 && SplitScreen::IsActive())
	{
		auto cam_twp = camera_twp;

		if (camera_twp)
		{
			LoopTaskC(tp);

			njControl3D_Backup();
			njControl3D_Add(NJD_CONTROL_3D_NO_CLIP_CHECK);
			njControl3D_Remove(NJD_CONTROL_3D_DEPTH_QUEUE);
			___njFogDisable();
			___njClipZ(gClipSky.f32Near, gClipSky.f32Far);
			if (cam_twp->pos.y > -10400.0f)
			{
				njPushMatrixEx();
				njTranslateEx(&cam_twp->pos);
				njScaleEx(&gSkyScale);
				njSetTexture(&bg_highway_TEXLIST);
				dsDrawModel(object_s1_nbg1_nbg1.basicdxmodel);
				njTranslate(0, 0.0f, -10000.0f - cam_twp->pos.y * 0.2f, 0.0f);
				njSetTexture(&bg_highway02_TEXLIST);
				dsDrawModel(object_s2_yakei_yakei.basicdxmodel);
				njPopMatrixEx();
			}
			___njClipZ(gClipMap.f32Near, gClipMap.f32Far);
			___njFogEnable();
			njControl3D_Restore();

			HighwayMaskBlock();
		}
	}
	else
	{
		dispBgHighway_h.Original(tp);
	}
}

void patch_rd_highway_init()
{
	RdHighwayCheckArriveAtTheBuilding_h.Hook(RdHighwayCheckArriveAtTheBuilding_r);
	subRd_Highway_h.Hook(subRd_Highway_r);
	dispBgHighway_h.Hook(dispBgHighway_r);

	WriteData((taskwk***)0x610765, &camera_twp); // Use camera instead of player to check when to hide skybox
	WriteData((void**)0x610A7E, (void*)0x6109E0); // Patch skybox mode
}

RegisterPatch patch_rd_highway(patch_rd_highway_init);