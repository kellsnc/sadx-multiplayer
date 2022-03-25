#include "pch.h"
#include "multiplayer.h"

DataPointer(NJS_ACTION, action_bowling_bowlingdai, 0x27C23FC);
TaskFunc(dispCatapult, 0x621FA0);

auto sub_622000 = GenerateUsercallWrapper<void (*)(taskwk* twp)>(noret, 0x622000, rECX);
auto rotateArms = GenerateUsercallWrapper<void (*)(taskwk* twp)>(noret, 0x621EF0, rEDI);
auto manipulateArm = GenerateUsercallWrapper<void (*)(taskwk* twp)>(noret, 0x6220F0, rEDI);

enum : char
{
    MODE_INIT,
    MODE_NORMAL,
    MODE_AIM,
    MODE_3,
    MODE_4,
    MODE_5
};

static void inhallPlayer_m(taskwk* twp)
{
    auto pnum = twp->id;
    auto ptwp = playertwp[pnum];

    if (ptwp)
    {
        NJS_POINT3 v = { ptwp->pos.x - twp->pos.x, ptwp->pos.y - twp->pos.y, ptwp->pos.z - twp->pos.z };
        njPushMatrix(_nj_unit_matrix_);
        njRotateY_(-twp->ang.y);
        njCalcVector(0, &v, &v);
        njPopMatrixEx();

        if (v.x <= 41.5f)
        {
            if (v.x < -41.5f)
            {
                v.x += 0.5f;
            }
        }
        else
        {
            v.x -= 0.5f;
        }

        if (v.y <= 0.0f)
        {
            v.y = 0.0f;
        }
        else
        {
            v.y *= 0.9f;
        }

        if (fabs(v.z) >= 0.5f)
        {
            v.z *= 0.75f;
        }
        else
        {
            v.z = 0.0f;
            twp->mode = MODE_4;
        }

        njPushMatrix(_nj_unit_matrix_);
        njRotateY_(twp->ang.y);
        njCalcVector(0, &v, &v);
        njPopMatrixEx();
        ptwp->pos.x = v.x + twp->pos.x;
        ptwp->pos.y = v.y + twp->pos.y;
        ptwp->pos.z = v.z = twp->pos.z;
        dsPlay_timer_v(69, (int)twp, 1, 0, 2, twp->pos.x, twp->pos.y, twp->pos.z);
        VibShot(0, 4);
    }
}

static void manipulateArm_m(taskwk* twp)
{
    auto ptwp = playertwp[GetClosestPlayerNum(&twp->pos)];
    NJS_POINT3 v = { ptwp->pos.x - twp->pos.x, 0.0f, ptwp->pos.z - twp->pos.z };
    
    njPushMatrix(_nj_unit_matrix_);
    njRotateY_(twp->ang.y);
    njCalcVector(0, &v, &v);
    njPopMatrixEx();

    auto obj = ((NJS_ACTION*)twp->timer.ptr)->object->child;
    auto sibling = obj->sibling;

    auto v3 = v.x - sibling->pos[0] - 9.0f;
    if (fabs(v3) >= 0.5f)
    {
        v3 = v3 * 0.25f + sibling->pos[0];
    }
    else
    {
        v3 = v.x - 9.0f;
    }

    sibling->pos[0] = v3;

    if (sibling->pos[0] + 18.0f > obj->pos[0])
    {
        sibling->pos[0] = obj->pos[0] - 18.0f;
    }

    if (sibling->pos[0] < -54.27f)
    {
        sibling->pos[0] = -54.27f;
    }

    v3 = v.x - obj->pos[0] + 9.0f;
    if (fabs(v3) >= 0.5)
    {
        v3 = v3 * 0.25f + obj->pos[0];
    }
    else
    {
        v3 = v.x + 9.0f;
    }

    obj->pos[0] = v3;

    if (obj->pos[0] - 18.0f < sibling->pos[0])
    {
        obj->pos[0] = sibling->pos[0] + 18.0f;
    }

    if (obj->pos[0] > 54.27f)
    {
        obj->pos[0] = 54.27f;
    }
}

static bool checkOnBoard_m(taskwk* twp)
{
    for (int i = 0; i < PLAYER_MAX; ++i)
    {
        auto ptwp = playertwp[i];

        if (!ptwp)
        {
            continue;
        }
        
        NJS_POINT3 v = { ptwp->pos.x - twp->pos.x, ptwp->pos.y - twp->pos.y, ptwp->pos.z - twp->pos.z };

        njPushMatrix(_nj_unit_matrix_);
        njRotateY_(-twp->ang.y);
        njCalcVector(0, &v, &v);
        njPopMatrixEx();

        if (fabs(v.x) < 54.27f && fabs(v.z) < 21.08f && fabs(v.y) < 50.0f)
        {
            twp->id = i;
            return true;
        }
    }

    return false;
}

static void Aim(taskwk* twp)
{
    auto pnum = IsPlayerInSphere(&twp->pos, 100.0f) - 1;

    if (pnum >= 0)
    {
        if (checkOnBoard_m(twp))
        {
            auto pnum = twp->id;
            twp->mode = MODE_3;
            SetInputP(pnum, 13);
            SetRotationP(pnum, 0, ((NJS_ACTION*)twp->timer.ptr)->object->child->child->ang[1] + twp->ang.y + 0x8000, 0);
        }
    }
    else
    {
        twp->mode = MODE_NORMAL;
    }

    rotateArms(twp);
    manipulateArm_m(twp);
}

static void NormalMove(taskwk* twp, NJS_OBJECT* obj, int flag)
{
    if (twp->btimer & flag)
    {
        if (obj->pos[0] > -54.27f)
        {
            obj->pos[0] -= 0.5f;
        }
        else
        {
            twp->btimer &= ~flag;
        }
    }
    else
    {
        if (obj->pos[0] < 54.27f)
        {
            obj->pos[0] += 0.5f;
        }
        else
        {
            twp->btimer |= flag;
        }
    }
}

static void Normal(taskwk* twp)
{
    if (IsPlayerInSphere(&twp->pos, 100.0f))
    {
        twp->mode = MODE_AIM;
    }

    sub_622000(twp);

    auto obj = ((NJS_ACTION*)twp->timer.ptr)->object->child;

    NormalMove(twp, obj, 2);
    NormalMove(twp, obj->sibling, 1);

    rotateArms(twp);
}

static void ObjectTPCatapult_m(task* tp)
{
    if (!CheckRangeOutWithR(tp, 372100.0f))
    {
        auto twp = tp->twp;

        switch (twp->mode)
        {
        case MODE_INIT:
            twp->mode = MODE_NORMAL;
            twp->scl.x += 6.0f;
            twp->btimer = twp->btimer & 0xFC | 1;
            twp->scl.y = 0.0;
            twp->timer.ptr = &action_bowling_bowlingdai;
            twp->scl.z = static_cast<Float>(action_bowling_bowlingdai.motion->nbFrame);
            tp->disp = dispCatapult;
            break;
        case MODE_NORMAL:
            Normal(twp);
            break;
        case MODE_AIM:
            Aim(twp);
            break;
        case MODE_3:
            inhallPlayer_m(twp);
            if (!checkOnBoard_m(twp))
            {
                twp->mode = 2;
                SetInputP(twp->id, 24);
            }
            break;
        case MODE_4:
            inhallPlayer_m(twp);
            if (checkOnBoard_m(twp))
            {
                if (per[twp->id]->press & (Buttons_A | Buttons_B))
                {
                    twp->mode = MODE_5;
                }
            }
            else
            {
                twp->mode = MODE_AIM;
                SetInputP(twp->id, 24);
            }
            rotateArms(twp);
            manipulateArm_m(twp);
            SetRotationP(twp->id, 0, ((NJS_ACTION*)twp->timer.ptr)->object->child->child->ang[1] + twp->ang.y + 0x8000, 0);
            break;
        case MODE_5:
            if (checkOnBoard_m(twp))
            {
                {
                    NJS_POINT3 p = { twp->scl.x, 0.0f, 0.0f };
                    Angle3 ang = { 0, ((NJS_ACTION*)twp->timer.ptr)->object->child->child->ang[1] + twp->ang.y + 0x8000, 0 };
                    SetVelocityAndRotationAndNoconTimeWithSpinDashP(twp->id, &p, &ang, 30);
                    dsPlay_oneshot(68, 0, 0, 0);
                }
            }
            else
            {
                twp->mode = MODE_AIM;
            }
            break;
        }

        if (twp->mode > 2)
        {
            twp->scl.y += 1.0f;
            if (twp->scl.y >= twp->scl.z)
            {
                twp->scl.y = 0.0f;
            }
        }

        tp->disp(tp);
    }
}

static void __cdecl ObjectTPCatapult_r(task* tp);
Trampoline ObjectTPCatapult_t(0x6223C0, 0x6223C8, ObjectTPCatapult_r);
static void __cdecl ObjectTPCatapult_r(task* tp)
{
    if (multiplayer::IsActive())
    {
        ObjectTPCatapult_m(tp);
    }
    else
    {
        TARGET_STATIC(ObjectTPCatapult)(tp);
    }
}