#include "pch.h"
#include "SADXModLoader.h"
#include "FunctionHook.h"
#include "multiplayer.h"

// Casinopolis object that sends the player in a parabolic manner with smoke effects (cannons, barrel)

DataPointer(CCL_INFO, object_c_info_40, 0x1E759C4);
DataPointer(NJS_POINT3, CasHosinVec, 0x1E759AC);
DataPointer(Float, CasHosinSmokeSpd, 0x1E759BC);
DataPointer(Float, CasHosinY, 0x1E759B8);
DataPointer(Float, CasHosinSmokeScl, 0x1E759C0);

void ExecHosin_m(task* tp)
{
    if (!CheckRangeOutWithR(tp, 62510.0f))
    {
        auto twp = tp->twp;

        if (twp->cwp->flag & 1)
        {
            auto ptwp = twp->cwp->hit_cwp->mytask->twp;
            auto pnum = TASKWK_PLAYERID(ptwp);
            auto ppwp = playerpwp[pnum];

            NJS_VECTOR vec, spd, dir;

            vec.x = CasHosinVec.x + twp->scl.y;
            vec.y = CasHosinVec.y;
            vec.z = CasHosinVec.z;
            spd.x = 0.0f;
            spd.y = CasHosinSmokeSpd;
            spd.z = 0.0f;
            dir.x = 0.0f;
            dir.y = object_c_info_40.b + 5.0f;
            dir.z = 0.0f;
            njUnitVector(&vec);

            njPushMatrix(_nj_unit_matrix_);
            njTranslateEx(&twp->pos);
            njRotateZ_(twp->ang.z);
            njRotateX_(twp->ang.x);
            njRotateY_(twp->ang.y);
            njCalcVector(0, &vec, &vec);
            njCalcVector(0, &spd, &spd);
            njCalcPoint(0, &dir, &dir);
            njPopMatrixEx();

            SetParabolicMotionP(pnum, twp->scl.z + CasHosinY, &vec);
            CreateSmoke(&dir, &spd, CasHosinSmokeScl);
            ppwp->nocontimer = 60;
            EntryColliList(twp);
        }
        else
        {
            twp->wtimer = 0;
            EntryColliList(twp);
        }
    }
}

void __cdecl ExecHosin_r(task* tp);
TaskHook ExecHosin_h(0x5C5400, ExecHosin_r);
void __cdecl ExecHosin_r(task* tp)
{
	if (multiplayer::IsActive)
	{
		ExecHosin_m(tp);
	}
	else
	{
		ExecHosin_h.Original(tp);
	}
}
