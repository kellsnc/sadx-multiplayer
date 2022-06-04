#include "pch.h"
#include "SADXModLoader.h"
#include "utils.h"
#include "multiplayer.h"

// Target mark for EnemyAir (e_aircraft)

static void  EffectSkyScope_m(task* tp)
{
    auto twp = tp->twp;
    if (twp->mode)
    {
        if (twp->mode == 1)
        {
            if (twp->btimer & 1)
            {
                auto pnum = twp->wtimer; // we store the player number here, defaults to 0 anyway 
                auto ptwp = playertwp[pnum];
                auto ppwp = playerpwp[pnum];
                
                if (ptwp && ppwp)
                {
                    twp->pos.x = (playertwp[pnum]->pos.x - twp->pos.x) * 0.25 + twp->pos.x;
                    twp->pos.z = (playertwp[pnum]->pos.z - twp->pos.z) * 0.25 + twp->pos.z;

                    auto ppwp = playerpwp[pnum];
                    auto shadowpos = ppwp->shadow.y_bottom;;
                    if (shadowpos != -1000000.0f)
                    {
                        twp->pos.y = shadowpos;
                        if (twp->btimer & 2)
                        {
                            twp->ang.x = ppwp->shadow.angx;
                            twp->ang.z = ppwp->shadow.angz;
                        }
                    }
                }
            }
            tp->disp(tp);
        }
    }
    else
    {
        twp->mode = 1;
        tp->disp = dispSkyScope;
        twp->counter.ptr = &object_sky_scope_a_scope_a;
    }
}

static void __cdecl EffectSkyScope_r(task* tp);
Trampoline EffectSkyScope_t(0x5F1AA0, 0x5F1AA6, EffectSkyScope_r);
static void __cdecl EffectSkyScope_r(task* tp)
{
    if (multiplayer::IsActive())
    {
        EffectSkyScope_m(tp);
    }
    else
    {
        TARGET_STATIC(EffectSkyScope)(tp);
    }
}