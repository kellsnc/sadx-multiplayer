#include "pch.h"
#include "multiplayer.h"
#include "splitscreen.h"

DataPointer(NJS_MATRIX, head_matrix, 0x3C53AD8); // static to E102.c

void __cdecl E102_r(task* tp);
Trampoline E102_t(0x47FD50, 0x47FD57, E102_r);
void __cdecl E102_r(task* tp)
{
    if (multiplayer::IsActive())
    {
        auto pwp = (playerwk*)tp->mwp->work.ptr;
        auto ewk = (E102WK*)pwp->free.ptr[0];

        // Recalculate submodel positions for splitscreen compatibility
        if (SplitScreen::IsActive())
        {
            e102_work_ptr = ewk;
            

            if (pwp->mj.mtnmode == 2)
            {
                CalcMMMatrix(_nj_unit_matrix_, pwp->mj.actwkptr, pwp->mj.nframe, pwp->mj.plactptr[pwp->mj.reqaction].objnum, nullptr);
            }
            else
            {
                CalcMMMatrix(_nj_unit_matrix_, pwp->mj.plactptr[pwp->mj.reqaction].actptr, pwp->mj.nframe, pwp->mj.plactptr[pwp->mj.reqaction].objnum, 0);
            }

            GetMMMatrix(54, head_matrix);
            GetMMMatrix(30, (NJS_MATRIX_PTR)0x3C53C48);
            GetMMMatrix(39, (NJS_MATRIX_PTR)0x3C53B78);
            njRotateZ((NJS_MATRIX_PTR)0x3C53B78, 0x4000);
            GetMMMatrix(2, (NJS_MATRIX_PTR)0x3C53B20);
            GetMMMatrix(37, (NJS_MATRIX_PTR)0x3C53C00);
        }

        // Hide normal arm *before* rendering the model...
        if ((ewk->gun_mode < 2 || ewk->gun_mode > 5) && (ewk->laser_mode < 2 || ewk->laser_mode > 3))
        {
            clrObjFlags(&e102_arm, 8u);
        }
        else
        {
            setObjFlags(&e102_arm, 8u);
        }
    }
    
	TARGET_STATIC(E102)(tp);
}

void __cdecl E102LockOnCursor_r(task* tp);
Trampoline E102LockOnCursor_t(0x4CF090, 0x4CF097, E102LockOnCursor_r);
void __cdecl E102LockOnCursor_r(task* tp)
{
    e102_work_ptr = (E102WK*)tp->awp[1].work.ul[0];
    TARGET_STATIC(E102LockOnCursor)(tp);
}