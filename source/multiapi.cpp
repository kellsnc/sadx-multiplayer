#include "pch.h"
#include "splitscreen.h"
#include "players.h"
#include "result.h"
#include "camera.h"
#include "gravity.h"
#include "collision.h"
#include "../include/multiapi.h"

bool splitscreen_is_active()
{
	return SplitScreen::IsActive();
}

bool viewport_is_enabled(int32_t num)
{
	return SplitScreen::IsScreenEnabled(num);
}

void viewport_save()
{
	SplitScreen::SaveViewPort();
}

void viewport_restore()
{
	SplitScreen::RestoreViewPort();
}

void viewport_change(int32_t num)
{
	SplitScreen::ChangeViewPort(num);
}

uint32_t viewport_get_num()
{
	return SplitScreen::GetCurrentScreenNum();
}

bool viewport_get_info(int32_t num, float* x, float* y, float* w, float* h)
{
	auto ratio = SplitScreen::GetScreenRatio(num);

	if (ratio)
	{
		*x = ratio->x;
		*y = ratio->y;
		*w = ratio->w;
		*h = ratio->h;

		return true;
	}

	return false;
}

void multi_set_charid(uint32_t pnum, Characters character)
{
	SetCurrentCharacter(pnum, character);
}

void multi_score_reset()
{
	ResetEnemyScoreM();
}

int32_t multi_score_get(uint32_t pnum)
{
	return GetEnemyScoreM(pnum);
}

void multi_score_add(uint32_t pnum, int32_t amount)
{
	AddEnemyScoreM(pnum, amount);
}

void multi_lives_reset()
{
	ResetNumPlayerM();
}

int32_t multi_lives_get(uint32_t pnum)
{
	return GetNumPlayerM(pnum);
}

void multi_lives_add(uint32_t pnum, int32_t amount)
{
	AddNumPlayerM(pnum, amount);
}

void multi_rings_reset(uint32_t pnum)
{
	ResetNumRingP(pnum);
}

int32_t multi_rings_get(uint32_t pnum)
{
	return GetNumRingM(pnum);
}

void multi_rings_add(uint32_t pnum, int32_t amount)
{
	AddNumRingM(pnum, amount);
}

bool multi_is_enabled()
{
	return multiplayer::IsEnabled();
}

bool multi_is_active()
{
	return multiplayer::IsActive();
}

bool multi_is_battle()
{
	return multiplayer::IsBattleMode();
}

bool multi_is_coop()
{
	return multiplayer::IsCoopMode();
}

bool multi_enable(uint32_t player_count, bool battle)
{
	return multiplayer::Enable(player_count, battle ? multiplayer::mode::battle : multiplayer::mode::coop);
}

bool multi_disable()
{
	return multiplayer::Disable();
}

uint32_t multi_get_player_count()
{
	return multiplayer::GetPlayerCount();
}

int32_t multi_get_winner()
{
	return GetWinnerMulti();
}

void multi_set_winner(uint32_t pnum)
{
	SetWinnerMulti(pnum);
}

bool multi_get_gravity(uint32_t pnum, NJS_POINT3* v, Angle* angx, Angle* angz)
{
	return gravity::GetUserGravity(pnum, v, angx, angz);
}

void multi_set_gravity(uint32_t pnum, Angle angx, Angle angz)
{
	gravity::SetUserGravity(angx, angz, pnum);
}

void multi_reset_gravity(uint32_t pnum)
{
	gravity::ResetUserGravity(pnum);
}

void camera_apply(uint32_t num)
{
	ApplyMultiCamera(num);
}

bool camera_get_pos(uint32_t num, NJS_POINT3* pos)
{
	return GetCameraPosition(num);
}

Angle3* camera_get_ang(uint32_t num)
{
	return GetCameraAngle(num);
}

void camera_set_pos(uint32_t num, float x, float y, float z)
{
	SetCameraPosition(num, x, y, z);
}

void camera_set_ang(uint32_t num, Angle x, Angle y, Angle z)
{
	SetCameraAngle(num, x, y, z);
}

uint32_t camera_get_fov(uint32_t num)
{
	return ds_GetPerspective_m(num);
}

void camera_set_fov(uint32_t num, Angle fov)
{
	njSetPerspective_m(num, fov);
}

void camera_enable_freecam(uint32_t pnum, bool enable)
{
	SetFreeCamera_m(pnum, enable);
}

bool camera_is_freecam_enabled(uint32_t pnum)
{
	return GetFreeCamera_m(pnum);
}

void camera_allow_freecam(uint32_t pnum, bool allow)
{
	SetFreeCameraMode_m(pnum, allow);
}

bool camera_is_freecam_allowed(uint32_t pnum)
{
	return GetFreeCameraMode_m(pnum);
}

void camera_set_normal_camera(uint32_t pnum, uint32_t ssCameraMode, uint32_t ucAdjustType)
{
	CameraSetNormalCamera_m(pnum, ssCameraMode, ucAdjustType);
}

void camera_set_event_camera(uint32_t pnum, uint32_t ssCameraMode, uint32_t ucAdjustType)
{
	CameraSetEventCamera_m(pnum, ssCameraMode, ucAdjustType);
}

void camera_set_event_camera_func(uint32_t pnum, CamFuncPtr fnCamera, uint32_t ucAdjustType, uint32_t scCameraDirect)
{
	CameraSetEventCameraFunc_m(pnum, fnCamera, ucAdjustType, scCameraDirect);
}

void camera_release_event_camera(uint32_t pnum)
{
	CameraReleaseEventCamera_m(pnum);
}

colaround* multi_get_enemy_list(Uint8 pno)
{
	return GetTargetEnemyList(pno);
}

Uint16* multi_get_enemy_list_index(Uint8 pno)
{
	return getAelNum(pno);
}