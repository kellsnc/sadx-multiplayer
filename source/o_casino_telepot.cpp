#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"

static void ObjectCasinoTelepotExec_r(task* tp);
FastFunctionHook<void, task*> ObjectCasinoTelepotExec_t(0x5D10C0);

static void ObjectCasinoTelepotExec_r(task* tp)
{
	if (!multiplayer::IsActive() || ObjectSelectedDebug((ObjectMaster*)tp))
	{
		return ObjectCasinoTelepotExec_t.Original(tp);
	}

	if (CheckRangeOutWithR(tp, 360010.0f))
		return;

	tp->disp(tp);

	taskwk* twp = tp->twp;

	auto pnum = GetTheNearestPlayerNumber(&twp->pos);
	auto pwp = playerpwp[pnum];

	if (!pwp)
		return;

    switch (twp->mode) {
    case 1:
        if (twp->cwp->flag & CWK_FLAG_HIT)
        {
            //Draw the player in
            njScalor(&pwp->spd);

            NJS_VECTOR spd = {
                twp->pos.x - playertwp[pnum]->pos.x,
                twp->pos.y - playertwp[pnum]->pos.y,
                twp->pos.z - playertwp[pnum]->pos.z,
            };
            spd.y -= 10.0f;
            pwp->nocontimer = 3;
            if (spd.z * spd.z + spd.x * spd.x >= 10.0f)
            {
                Float movespd = SAL_telepotsonicmovespd / njScalor(&spd);
                spd.x *= movespd;
                spd.y *= movespd;
                spd.z *= movespd;
                SetRotationP(pnum, 0, 0xC000 - njArcTan2(spd.z, spd.x), 0);
                SetVelocityP(pnum, spd.x, spd.y, spd.z);
                EntryColliList(twp);
                ++twp->wtimer;
                return;
            }
            if (!teleporttask)
            {
                teleporttask = tp;
            }
            twp->mode = 2;
        }
        else
        {
            twp->mode = 0;
        }
        break;
    case 2:
        if (twp->cwp->flag & CWK_FLAG_HIT)
        {
            pwp->nocontimer = 3;
            VibShot(pnum, 0);
            dsPlay_timer(SE_CA_TRANS, (Sint32)twp, 1, 0, 5);
            SetVelocityP(pnum, 0.0f, 0.0f, 0.0f);
            pwp->acc.y += pwp->p.weight + 0.1f; //Make the player float.

            Float rot = (Float)(SAL_telepotsonicrotspd >> 16);
            if (twp->wtimer <= 30)
            {
                rot *= (Float)twp->wtimer * 0.025f;
            }
            else
            {
                rot *= 1.0f;
            }

            playertwp[pnum]->ang.y += ((Sint16)SAL_telepotsonicrotspd | ((Sint16)rot << 8));
            if (twp->value.w[1] == GetStageNumber())
            {
                lig_fillOffsetPalette(2, (2 * twp->wtimer) | (((2 * twp->wtimer) | (((2 * twp->wtimer) | -0x100) << 8)) << 8), 1);
            }
            if (twp->wtimer == 100)
            {
                RdCasino_JumpAct = (Sint32)twp->scl.x;
                EntryColliList(twp);
                ++twp->wtimer;
                return;
            }
        }
        else
        {
            lig_fillOffsetPalette(2, 0xFF000000, 1);
            twp->mode = 0;
            twp->wtimer = 0;
            if (teleporttask == tp)
            {
                teleporttask = 0;
                EntryColliList(twp);
                ++twp->wtimer;
                return;
            }
        }
        EntryColliList(twp);
        ++twp->wtimer;
        return;
    default:
        if (twp->wtimer > 150)
        {
            twp->wtimer = 0;
        }
        if (!(twp->cwp->flag & CWK_FLAG_HIT))
        {
            EntryColliList(twp);
            ++twp->wtimer;
            return;
        }
        twp->mode = 1;
        break;
    }
    twp->wtimer = 0;
    EntryColliList(twp);
    ++twp->wtimer;
}

void patch_casino_telepot_init()
{
    ObjectCasinoTelepotExec_t.Hook(ObjectCasinoTelepotExec_r);
}

RegisterPatch patch_casino_telepot(patch_casino_telepot_init);