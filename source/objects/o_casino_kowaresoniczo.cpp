#include "pch.h"
#include "SADXModLoader.h"
#include "Trampoline.h"
#include "multiplayer.h"
#include "camera.h"
#include "camerafunc.h"

// Fix Casinopolis statue camera trigger
// Everything got inlined so a full rewrite is necessary

enum : Sint8
{
    MODE_INIT,
    MODE_CRASH,
    MODE_OUT // custom
};

// custom
struct soniczonode
{
    NJS_POINT3 pos;
    NJS_POINT3 spd;
    Angle pitch;
    Angle yaw;
    Angle yaw_spd;
    Angle pitch_spd;
};

// custom
struct soniczowork
{
    Sint32 count;
    soniczonode* models; // 168
};

DataArray(NJS_OBJECT*, psoniczo, 0x1E74C60, 168);
DataArray(CCL_INFO, object_c_info, 0x1E74F38, 7);

static void Crash_m(task* tp)
{
    auto twp = tp->twp;
    auto wk = reinterpret_cast<soniczowork*>(twp->value.ptr);

    ObjectCasinoTutuAEnd(tp);

    twp->cwp->info->attr |= 0x10;
    twp->cwp->info[1].attr &= ~0x10;
    twp->cwp->info[2].attr |= 0x10;
    twp->cwp->info[3].attr |= 0x10;
    twp->cwp->info[4].attr |= 0x10;
    twp->cwp->info[5].attr |= 0x10;
    twp->cwp->info[6].attr |= 0x10;

    int last;

    if (twp->wtimer < SAL_kowarezocrashtimecomptime)
    {
        Float t = (Float)twp->wtimer / (Float)SAL_kowarezocrashtimecomptime;
        last = (Sint32)((Float)wk->count * t);
        auto model = &wk->models[last];
        CreateCrashSmoke(twp, &model->pos, SAL_kowarezocrashsmokescl);
        twp->cwp->info[1].a = object_c_info[1].a * t;
    }
    else
    {
        last = wk->count;
        twp->cwp->info[1].a = object_c_info[1].a;
    }

    if (last)
    {
        Float floor_level = wk->models[0].pos.y;

        for (int i = 0; i < last; ++i)
        {
            auto model = &wk->models[i];

            model->pos.x += model->spd.x;
            model->pos.y += model->spd.y;
            model->pos.z += model->spd.z;

            if (model->pos.y < floor_level)
            {
                Float mag = njScalor(&model->spd);

                if (mag <= SAL_kowarezocrashrefbdspd)
                {
                    model->pos.y = floor_level;
                    model->spd.x = 0.0f;
                    model->spd.y = 0.0f;
                    model->spd.z = 0.0f;
                    model->yaw_spd *= SAL_kowarezocrashrefspdmul;
                    model->pitch_spd *= SAL_kowarezocrashrefspdmul;
                }
                else
                {
                    if (!++twp->btimer)
                    {
                        dsPlay_oneshot(277, 0, 0, 0);
                    }

                    if (twp->btimer > 20)
                    {
                        twp->btimer = 0;
                    }

                    model->pos.y = floor_level;

                    model->spd.x *= SAL_kowarezocrashrefspdmul;
                    model->spd.y *= SAL_kowarezocrashrefspdmul;
                    model->spd.z *= SAL_kowarezocrashrefspdmul;

                    model->spd.x += ((njRandom() - 0.5f) * SAL_kowarezocrashrefspd) * 2;
                    model->spd.y += ((njRandom() - 0.5f) * SAL_kowarezocrashrefspd) * 2;
                    model->spd.z += ((njRandom() - 0.5f) * SAL_kowarezocrashrefspd) * 2;

                    model->yaw_spd *= SAL_kowarezocrashrefspdmul;
                    model->pitch_spd *= SAL_kowarezocrashrefspdmul;

                    CreateCrashSmoke(twp, &model->pos, SAL_kowarezohitsmokesclmul * mag);
                    VibShot(twp->smode, 0);
                }
            }

            model->spd.y += SAL_kowarezofallspeed;

            model->spd.x *= SAL_kowarezofraction;
            model->spd.y *= SAL_kowarezofraction;
            model->spd.z *= SAL_kowarezofraction;

            model->yaw += model->yaw_spd;
            model->pitch += model->pitch_spd;

            model->yaw_spd *= SAL_kowarezofraction;
            model->pitch_spd *= SAL_kowarezofraction;
        }
    }

    if (twp->wtimer > SAL_kowarezocrashtime)
    {
        twp->cwp->info[1].attr |= 0x10u;
        CameraReleaseEventCamera_m(twp->smode);
        CamPathCam2Core_AliveFlag_m[twp->smode] = 0;
        twp->wtimer = 0;
        twp->mode = MODE_OUT;

        if (twp->scl.x != 0.0f || twp->scl.y != 0.0f || twp->scl.z != 0.0f)
        {
            PositionPlayer(twp->smode, twp->scl.x, twp->scl.y, twp->scl.z);
        }
    }
}

static void Normal_m(task* tp)
{
    auto twp = tp->twp;

    ObjCasino_ExecGColl2(tp, 200.0f, ((NJS_OBJECT*)0x1E5F9C8)->getbasicdxmodel());

    if (twp->flag & Status_Hurt)
    {
        auto player = CCL_IsHitPlayer(twp);

        if (player)
        {
            auto pnum = TASKWK_PLAYERID(player);

            if (player->flag & Status_Ground && !IsEventCamera_m(pnum) && !IsCompulsionCamera_m(pnum))
            {
                twp->mode = MODE_CRASH;
                twp->wtimer = 0;
                twp->btimer = 0;
                twp->smode = pnum;

                auto wk = reinterpret_cast<soniczowork*>(twp->value.ptr);

                for (int i = 0; i < wk->count; ++i)
                {
                    auto model = &wk->models[i];
                    auto ref_model = psoniczo[i];

                    model->pos.x = ref_model->pos[0];
                    model->pos.y = ref_model->pos[1];
                    model->pos.z = ref_model->pos[2];

                    Float rad = SAL_kowarezocrashradspd / SAL_kowarezocrashrad;

                    model->spd.x = (model->pos.x * rad) + ((njRandom() - 0.5f) * SAL_kowarezocrashspd) * 2;
                    model->spd.y = (model->pos.y * rad) + ((njRandom() - 0.5f) * SAL_kowarezocrashspd) * 2;
                    model->spd.z = (model->pos.z * rad) + ((njRandom() - 0.5f) * SAL_kowarezocrashspd) * 2;

                    model->pitch_spd = NJM_DEG_ANG(((njRandom() - 0.5f) * SAL_kowarezocrashrotspd) * 2);
                    model->yaw_spd = NJM_DEG_ANG(((njRandom() - 0.5f) * SAL_kowarezocrashrotspd) * 2);

                    model->pitch = 0;
                    model->yaw = 0;
                }

                NJS_POINT3 vel = { 0.0f, 2.0f, 1.0f };
                NJS_POINT3 pos = { 221.0f, -173.0f, 97.0f };
                NJS_POINT3 tgt = { -25.0f, -60.0f, -14.5f };

                njPushMatrix(_nj_unit_matrix_);
                njTranslateEx(&twp->pos);
                njRotateY_(twp->ang.y);
                njCalcPoint(0, &pos, &pos);
                njCalcPoint(0, &tgt, &tgt);
                njCalcVector(0, &vel, &vel);
                njPopMatrixEx();

                auto x = pos.x - tgt.x;
                auto z = pos.z - tgt.z;

                VibShot(pnum, 2);
                dsPlay_oneshot(240, 0, 0, 0);
                CameraSetEventCameraFunc_m(pnum, PathCamera2Core, 0, 0);
                CamPathCam2Core_Pos_m[pnum] = pos;
                CamPathCam2Core_AliveFlag_m[pnum] = 0;
                CamPathCam2Core_Angle_m[pnum].x = njArcTan2(tgt.y - pos.y, njSqrt(x * x + z * z));
                CamPathCam2Core_Angle_m[pnum].y = njArcTan2(x, z);
                CamPathCam2Core_Angle_m[pnum].z = 0;
                SetVelocityP(pnum, vel.x, vel.y, vel.z);
            }
        }
    }
}

static void ObjectCasinoCrashSoniczoExec_m(task* tp)
{
	auto twp = tp->twp;

    if (!IsPlayerInSphere(&twp->pos, 360000.0f))
    {
        return;
    }

    switch (twp->mode)
    {
    case MODE_INIT:
        Normal_m(tp);
        break;
    case MODE_CRASH:
        Crash_m(tp);
        break;
    case MODE_OUT:
        ObjectCasinoTutuAEnd(tp);
        //++twp->mode; <-- Either on purpose or not, this prevents the debris field from drawing
        break;
    }

    EntryColliList(twp);

    if (twp->flag & 0x100)
    {
        if (!IsPlayerInSphere(twp->pos.x, twp->pos.y - 119.0f, twp->pos.z, 110.0f))
        {
            twp->flag &= ~0x100;
        }
    }
    if (!++twp->wtimer)
    {
        twp->wtimer = -1;
    }

    tp->disp(tp);
}

static void __cdecl ObjectCasinoCrashSoniczoExec_r(task* tp);
Trampoline ObjectCasinoCrashSoniczoExec_t(0x5C3900, 0x5C3905, ObjectCasinoCrashSoniczoExec_r);
static void __cdecl ObjectCasinoCrashSoniczoExec_r(task* tp)
{
	if (multiplayer::IsEnabled())
	{
        ObjectCasinoCrashSoniczoExec_m(tp);
	}
	else
	{
		TARGET_STATIC(ObjectCasinoCrashSoniczoExec)(tp);
	}
}
