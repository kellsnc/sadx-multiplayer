#include "pch.h"
#include "SADXModLoader.h"
#include "Trampoline.h"
#include "utils.h"
#include "camera.h"
#include "splitscreen.h"

enum : __int8
{
    MODE_INITIAL,
    MODE_NORMAL,
    MODE_END
};

static void RotateObj_m(task* tp)
{
    auto twp = tp->twp;

    if (twp->mode == MODE_INITIAL)
    {
        twp->mode = 1i8;

        twp->pos.x = 0.0f;
        twp->pos.z = 0.0f;

        Float random = njRandom();
        twp->scl.y = njAbs(random);
        twp->counter.f = (random * 360.0f);

        tp->disp = (TaskFuncPtr)0x4DF500;
    }
    else if (twp->mode == MODE_NORMAL)
    {
        if (SplitScreen::IsVisible(&twp->pos, 10.0f))
        {
            twp->counter.f += twp->scl.y * 1.5f + 8.0f;
            Angle ang = NJM_DEG_ANG(-twp->counter.f);
            twp->pos.x = njCos(ang) * 100.0f;
            twp->pos.z = njSin(ang) * 100.0f;

            twp->pos.y = fabs(njRandom()) * 3.4f + twp->scl.y * 1.3f + twp->pos.y;

            ang = -NJM_DEG_ANG(twp->scl.y + twp->scl.y);
            twp->ang.x -= ang;
            twp->ang.z -= ang;

            twp->ang.y += NJM_DEG_ANG(twp->scl.y * 5.0f);
        }
    }

    tp->disp(tp);
}

static void __cdecl RotateObj_r(task* tp);
Trampoline RotateObj_t(0x4DF5A0, 0x4DF5A5, RotateObj_r);
static void __cdecl RotateObj_r(task* tp)
{
    if (multiplayer::IsActive())
    {
        RotateObj_m(tp);
    }
    else
    {
        TARGET_STATIC(RotateObj)(tp);
    }
}
