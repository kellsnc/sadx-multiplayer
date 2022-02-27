#include "pch.h"
#include "multiplayer.h"
#include "splitscreen.h"

// Gamma's laser doesn't have a display routine
// Let's copy the drawing part of the exec routine in the display one

DataArray(uint8_t, color_tbl_0, 0x98204C, 6);

static void __cdecl dispE102LaserDraw(task* tp)
{
    if (!MissedFrames)
    {
        auto twp = tp->twp;
        auto ewk = (E102WK*)twp->value.ptr;

        njPushMatrixEx();
        njTranslateEx(&twp->pos);
        njRotateY_(twp->ang.y);
        njRotateZ_(twp->ang.z);
        njColorBlendingMode(NJD_SOURCE_COLOR, NJD_COLOR_BLENDING_ONE);
        njColorBlendingMode(NJD_DESTINATION_COLOR, NJD_COLOR_BLENDING_ONE);

        auto color_num = color_tbl_0[twp->counter.b[0]];
        auto itensity = (float)((60 - ewk->laser_tmr) & ((60 - ewk->laser_tmr <= 0) - 1)) * 0.016666668f;

        e102laser_color[0].argb.r = (1.0 - itensity) * color_num;
        e102laser_color[1].argb.r = e102laser_color[0].argb.r;
        e102laser_color[0].argb.b = itensity * color_num;
        e102laser_color[1].argb.b = e102laser_color[0].argb.b;

        auto start = 0.0f;
        do
        {
            e102laser_pos[0].x = start;

            start += 7.5f;

            if (start < twp->timer.f)
            {
                start = twp->timer.f;
            }

            e102laser_pos[1].x = start;
            late_DrawLine3D(&e102laser_p3c, 1, 0x40u, 4);
        }
        while (start < twp->timer.f);

        ghDefaultBlendingMode();
        njPopMatrixEx();
    }
}

static void __cdecl dispE102Laser_r(task* tp);
Trampoline dispE102Laser_t(0x4C4C20, 0x4C4C25, dispE102Laser_r);
static void __cdecl dispE102Laser_r(task* tp)
{
    if (SplitScreen::IsActive())
    {
        auto twp = tp->twp;

        if (twp->mode == 0)
        {
            tp->disp = dispE102LaserDraw;
        }
    }

    TARGET_STATIC(dispE102Laser)(tp);
}