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

#pragma region drawEffectChaos0EffectB
void __cdecl drawEffectChaos0EffectBDestroy(task* tp)
{
    tp->awp = nullptr;
}

void __cdecl drawEffectChaos0EffectBDisplay(task* tp)
{
    if (!loop_count)
    {
        NJS_ARGB color = { 0.5f, 0.5f, 0.5f, 0.5f };
        ___njSetConstantMaterial(&color);
        njColorBlendingMode(NJD_SOURCE_COLOR, NJD_COLOR_BLENDING_ONE);
        njColorBlendingMode(NJD_DESTINATION_COLOR, NJD_COLOR_BLENDING_ONE);
        njSetTexture(&CHAOS_EFFECT_TEXLIST);
        
        for (int i = 0; i < reinterpret_cast<int>(tp->awp); ++i)
        {
            auto ctwp = (taskwk*)BigEntityArray[i];
            njPushMatrixEx();
            njTranslateV(0, &ctwp->pos);
            njRotateY_(camera_twp->ang.y);
            njRotateX_(camera_twp->ang.x);
            njScale(0, ctwp->scl.x, ctwp->scl.x, ctwp->scl.x);
            late_DrawSprite3D((NJS_SPRITE*)0x3D0D714, ctwp->counter.f, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR, LATE_LIG);
            njPopMatrixEx();
        }

        ResetMaterial();
        njColorBlendingMode(NJD_SOURCE_COLOR, NJD_COLOR_BLENDING_SRCALPHA);
        njColorBlendingMode(NJD_DESTINATION_COLOR, NJD_COLOR_BLENDING_INVSRCALPHA);
    }
}

void __cdecl drawEffectChaos0EffectB_r(task* tp)
{
    if (!tp->awp)
    {
        tp->awp = reinterpret_cast<anywk*>(BigEntityArrayLength);
        tp->dest = drawEffectChaos0EffectBDestroy;
        tp->disp = drawEffectChaos0EffectBDisplay;
    }

    tp->disp(tp);

    BigEntityArrayLength = 0;
    FreeTask(tp);
}
#pragma endregion

#pragma region drawEffectChaos0LightParticle
void __cdecl drawEffectChaos0LightParticleDestroy(task* tp)
{
    tp->awp = nullptr;
}

void __cdecl drawEffectChaos0LightParticleDisplay(task* tp)
{
    if (!loop_count)
    {
        njColorBlendingMode(NJD_SOURCE_COLOR, NJD_COLOR_BLENDING_ONE);
        njColorBlendingMode(NJD_DESTINATION_COLOR, NJD_COLOR_BLENDING_ONE);
        njSetTexture(&CHAOS_EFFECT_TEXLIST);

        for (int i = 0; i < reinterpret_cast<int>(tp->awp); ++i)
        {
            auto ctwp = (taskwk*)BigEntityArrayB[i];
            njPushMatrixEx();

            Float blend = max(0.0f, min((ctwp->timer.f - 0.25f) * 2, 1.0f));
           
            NJS_ARGB color;

            color.r = 0.5f;
            color.g = blend * 0.5f + 0.25f;
            color.b = blend * 0.098f + 0.25f;
            color.a = 0.75f - 0.5f * blend;

            ___njSetConstantMaterial(&color);

            njTranslateV(0, &ctwp->pos);
            njRotateY_(camera_twp->ang.y);
            njRotateX_(camera_twp->ang.x);
            njScale(0, ctwp->scl.x, ctwp->scl.x, ctwp->scl.x);
            late_DrawSprite3D((NJS_SPRITE*)0x3D0D73C, ctwp->counter.f, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR, LATE_LIG);
            njPopMatrixEx();
        }

        ResetMaterial();
        njColorBlendingMode(NJD_SOURCE_COLOR, NJD_COLOR_BLENDING_SRCALPHA);
        njColorBlendingMode(NJD_DESTINATION_COLOR, NJD_COLOR_BLENDING_INVSRCALPHA);
    }
}

void __cdecl drawEffectChaos0LightParticle_r(task* tp)
{
    if (!tp->awp)
    {
        tp->awp = reinterpret_cast<anywk*>(BigEntityArrayBLength);
        tp->dest = drawEffectChaos0LightParticleDestroy;
        tp->disp = drawEffectChaos0LightParticleDisplay;
    }

    tp->disp(tp);

    BigEntityArrayBLength = 0;
    FreeTask(tp);
}
#pragma endregion

#pragma region dispEffectChaos0AttackA
void __cdecl dispEffectChaos0AttackADisplay(task* tp)
{
    if (!loop_count)
    {
        auto twp = tp->twp;

        if (twp->counter.f < 15.0f)
        {
            NJS_ARGB color = { 0.5f, -0.5f, -0.5f, -0.5f };
            ___njSetConstantMaterial(&color);

            njColorBlendingMode(NJD_SOURCE_COLOR, NJD_COLOR_BLENDING_ONE);
            njColorBlendingMode(NJD_DESTINATION_COLOR, NJD_COLOR_BLENDING_ONE);
            njSetTexture(&CHAOS_EFFECT_TEXLIST);

            njPushMatrixEx();
            njTranslateV(0, &twp->pos);
            njRotateY_(camera_twp->ang.y);
            njRotateX_(camera_twp->ang.x);
            auto scl = twp->scl.x + 0.5f;
            njScale(0, scl, scl, scl);
            late_DrawSprite3D((NJS_SPRITE*)0x3D0D56C, twp->counter.f, NJD_SPRITE_ALPHA | NJD_SPRITE_COLOR, LATE_LIG);
            njPopMatrixEx();

            ResetMaterial();
            njColorBlendingMode(NJD_SOURCE_COLOR, NJD_COLOR_BLENDING_SRCALPHA);
            njColorBlendingMode(NJD_DESTINATION_COLOR, NJD_COLOR_BLENDING_INVSRCALPHA);
        }
    }
}

void __cdecl dispEffectChaos0AttackA_r(task* tp)
{
    if (!tp->disp)
    {
        tp->disp = dispEffectChaos0AttackADisplay;
    }

    auto twp = tp->twp;

    twp->counter.f += 0.75f;

    if (twp->counter.f < 15.0f)
    {
        twp->pos = *reinterpret_cast<NJS_POINT3*>(twp->timer.ptr);
    }
    else
    {
        FreeTask(tp);
    }

    tp->disp(tp);
}
#pragma endregion

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
	WriteJump(drawEffectChaos0EffectB, drawEffectChaos0EffectB_r);
	WriteJump(drawEffectChaos0LightParticle, drawEffectChaos0LightParticle_r);
	WriteJump(dispEffectChaos0AttackA, dispEffectChaos0AttackA_r);
	turnToPlayer_t.Hook(turnToPlayer_r);
	chaos0_t.Hook(Chaos0_Exec_r);
    Bg_Chaos0_t.Hook(Bg_Chaos0_r);
	setApartTargetPos_t.Hook(setApartTargetPos_r);
    chaos0Pole_t.Hook(chaos0Pole_r);
    chaos0Punch_t.Hook(chaos0Punch_r);
}