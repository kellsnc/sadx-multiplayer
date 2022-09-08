#include "pch.h"
#include "SADXModLoader.h"
#include "FunctionHook.h"
#include "UsercallFunctionHandler.h"
#include "camera.h"
#include "camerafunc.h"
#include "splitscreen.h"
#include "bosses.h"

static const int timeLimit = 300;

static FunctionHook<void, taskwk*, chaoswk*> turnToPlayer_t(0x7AE660);
static FunctionHook<void, task*> chaos0_t(0x548640);
static FunctionHook<void, task*> Bg_Chaos0_t(0x545DF0);
static UsercallFuncVoid(SetChaos0LandFlags, (BOOL flag), (flag), 0x5485E0, rEDX);
static UsercallFunc(Angle, setApartTargetPos_t, (chaoswk* cwk), (cwk), 0x546460, rEAX, rEDI);
static UsercallFuncVoid(chaos0Pole_t, (chaoswk* cwk, taskwk* data), (cwk, data), 0x547260, rECX, rESI);
static UsercallFuncVoid(chaos0Punch_t, (chaoswk* cwk, taskwk* data, bosswk* bwk), (cwk, data, bwk), 0x546790, rEDI, rESI, stack4);

//Patches Chaos effects to make them display on other player screens, it is done by manually setting a disp function that is lacking in vanilla.

void __cdecl ExecEffectChaos0AttackB(task* obj)
{
	SetAndDisp(obj, drawEffectChaos0EffectB);
}

void __cdecl ExecEffectChaos0LightParticleB(task* obj)
{
	SetAndDisp(obj, drawEffectChaos0LightParticle);
}

void __cdecl ExecEffectChaos0AttackA(task* obj)
{
	SetAndDisp(obj, dispEffectChaos0AttackA);
}

//Some Chaos effects don't have the flag data1 so we add it first
task* LoadChaos0AttackEffB(LoadObj flags, int index, void(__cdecl* loadSub)(task*))
{
	return CreateElementalTask(LoadObj_Data1, index, ExecEffectChaos0AttackB);
}

task* LoadChaos0LightParticleB(LoadObj flags, int index, void(__cdecl* loadSub)(task*))
{
	return CreateElementalTask(LoadObj_Data1, index, ExecEffectChaos0LightParticleB);
}

// series of hacks to make Chaos able to attack other players

void turnToPlayer_r(taskwk* twp, chaoswk* bwp)
{
	if (!multiplayer::IsActive())
	{
		return turnToPlayer_t.Original(twp, bwp);
	}

	float dist = 0.0f;

	if (playertwp[randomPnum])
	{
		dist = atan2((float)(playertwp[randomPnum]->pos.z - twp->pos.z), (float)(playertwp[randomPnum]->pos.x - twp->pos.x));
		twp->ang.y = AdjustAngle(
			twp->ang.y,
			(int)(dist * 65536.0f * 0.1591549762031479f) - bwp->attack_yang,
			chaosparam->turn_spd);
	}
}

Angle setApartTargetPos_r(chaoswk* cwk)
{
	if (!multiplayer::IsEnabled() || !playertwp[randomPnum])
	{
		return setApartTargetPos_t.Original(cwk);
	}

	Angle cos = 0;
	float sin = 0.0f;
	Angle result = 0;

	cos = (unsigned __int64)(atan2(
		chaosparam->field_center_pos.z - playertwp[randomPnum]->pos.z,
		chaosparam->field_center_pos.x - playertwp[randomPnum]->pos.x)
		* 65536.0f
		* 0.1591549762031479f);

	cwk->tgtpos.x = njCos(cos) * 60.0f + chaosparam->field_center_pos.x;
	sin = njSin(cos);
	result = cos;
	cwk->tgtpos.z = sin * 60.0f + chaosparam->field_center_pos.z;

	return result;
}

void chaos0Pole_r(chaoswk* cwk, taskwk* data)
{
    if (!multiplayer::IsEnabled() || !playertwp[randomPnum])
    {
        return chaos0Pole_t.Original(cwk, data);
    }

   float diffX = 0.0f;
   float diffZ = 0.0f;
   _BOOL1 isMode6 = false;

    auto player = playertwp[randomPnum];

    if (data->smode)
    {
        if (data->smode != 1)
        {
            return;
        }
    }
    else
    {
        cwk->bwk.req_action = MD_CHAOS_STND;
        bossmtn_flag &= 0xFDu;
        if (data->counter.b[0] == 2)
        {
            chaos_pole_punch_num = 5;
            data->counter.b[0] = 3;
        }
        data->smode = MD_CHAOS_STND;
    }

    turnToPlayer_r(data, cwk);

    if (chaos_pole_punch_num > 0)
    {
        diffX = player->pos.x - data->pos.x;
        diffZ = player->pos.z - data->pos.z;

        if (diffZ * diffZ + diffX * diffX >= 6400.0f)
        {
            if (data->mode != 4)
            {
                chaos_reqmode = 4;
                chaos_nextmode = data->mode;
                chaos_oldmode = chaos_nextmode;
            }
            data->counter.b[0] = 3;
        }
        else
        {
            chaos_punch_num = MD_CHAOS_STND;
            isMode6 = data->mode == 6;
            --chaos_pole_punch_num;
            if (!isMode6)
            {
                chaos_reqmode = 6;
                chaos_nextmode = data->mode;
                chaos_oldmode = chaos_nextmode;
            }
        }
    }
    else
    {
        if (data->mode != 4)
        {
            chaos_reqmode = 4;
            chaos_nextmode = data->mode;
            chaos_oldmode = chaos_nextmode;
        }

        data->counter.b[0] = MD_CHAOS_STND;
    }
}

void chaos0Punch_r(chaoswk* cwk, taskwk* data, bosswk* bwk)
{
    auto player = playertwp[randomPnum];
    float X = 0.0f;
    float Z = 0.0f;
    float calcZ = 0.0f;
    float calcY = 0.0f;
    float calcX = 0.0f;
    float result = 0.0f;
    float calcDist = 0.0f;

    if (data->smode != 3) 
    {
        return chaos0Punch_t.Original(cwk, data, bwk);
    }
    else
    {
        if (data->wtimer > 6)
        {
            turnToPlayer_r(data, cwk);
            X = player->pos.x - data->pos.x;
            Z = player->pos.z - data->pos.z;
            calcDist = Z * Z + X * X;
            cwk->attack_zang = (unsigned __int64)(atan2(player->pos.y - data->pos.y, squareroot(calcDist))
                * 65536.0f
                * -0.1591549762031479f);

            calcZ = data->pos.z - player->pos.z;
            calcY = data->pos.y - player->pos.y;
            calcX = data->pos.x - player->pos.x;
            result = calcX * calcX + calcY * calcY + calcZ * calcZ;
            cwk->attack_dist = squareroot(result);
          
            SetEffectPunchTameParticle((NJS_POINT3*)0x3C63DDC, 0);
        }

        if (!--data->wtimer)
        {
            bossmtn_flag &= 0xFCu;
            bwk->work.l = 0;
            data->smode = 4;
            cwk->etcflag |= 2u;
            setDrop(data, 10, 0.5f, 2.0f);
            setShakeHeadParam(1.5f);
            dsPlay_oneshot_Dolby(334, 0, 0, 207, 120, data);
            flt_3C63CEC = 2.9166667f / cwk->attack_dist;

            if (data->pos.y > 60.0f)
            {
                flt_3C63CEC = flt_3C63CEC * 1.5;
            }
        }
    }
}

void Bg_Chaos0_r(task* tp)
{
    if (!tp->disp)
    {
        tp->disp = Bg_Chaos0_r;
    }

    SetChaos0LandFlags(GetCameraMode_m(SplitScreen::GetCurrentScreenNum()) != CAMMD_CHAOS_P);
    Bg_Chaos0_t.Original(tp);
}

void Chaos0_Exec_r(task* tp)
{
    auto twp = tp->twp;

	if (twp && !twp->mode)
		ResetBossRNG();

    auto wk = (chaoswk*)tp->awp;

    if (!twp || !wk)
    {
        SetDefaultNormalCameraMode(CAMMD_CHAOS, CAMADJ_NONE);
        chaos0_t.Original(tp);
        return;
    }

    auto oldcam = wk->camera_mode;

	chaos0_t.Original(tp);

    if (oldcam != wk->camera_mode)
    {
        for (int i = 1; i < PLAYER_MAX; ++i)
        {
            CameraSetEventCamera_m(i, wk->camera_mode, 0);
        }
    }

	Boss_SetNextPlayerToAttack(timeLimit);
}

void initChaos0Patches()
{
	WriteCall((void*)0x7AD3F3, LoadChaos0AttackEffB);	
	WriteCall((void*)0x7AD388, LoadChaos0LightParticleB);	
	WriteCall((void*)0x7AD75D, LoadChaos0LightParticleB);
	WriteData((TaskFuncPtr*)0x7AD221, ExecEffectChaos0AttackA);
	turnToPlayer_t.Hook(turnToPlayer_r);
	chaos0_t.Hook(Chaos0_Exec_r);
    Bg_Chaos0_t.Hook(Bg_Chaos0_r);
	setApartTargetPos_t.Hook(setApartTargetPos_r);
    chaos0Pole_t.Hook(chaos0Pole_r);
    chaos0Punch_t.Hook(chaos0Punch_r);
}