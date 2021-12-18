#include "pch.h"
#include "camera.h"

Trampoline* PGetRotation_t = nullptr;
Trampoline* GetPlayersInputData_t = nullptr;

void __cdecl PGetRotation_r(taskwk* twp, motionwk2* mwp, playerwk* pwp)
{
    if (!IsIngame())
    {
        return 	TARGET_DYNAMIC(PGetRotation)(twp, mwp, pwp);
    }

	auto backup = camera_twp->ang;
	camera_twp->ang = *GetCameraAngle(TASKWK_PLAYERID(twp));
	TARGET_DYNAMIC(PGetRotation)(twp, mwp, pwp);
	camera_twp->ang = backup;
}

void __cdecl GetPlayersInputData_r()
{
	for (int i = 0; i < PLAYER_MAX; ++i)
	{
        auto controller = per[i];
        float lx = (controller->x1 << 8); // left stick x
        float ly = (controller->y1 << 8); // left stick y

        int ang;
        float strk;

        if (lx > 3072.0f || lx < -3072.0f || ly > 3072.0f || ly < -3072.0f)
        {
            lx = lx <= 3072.0f ? (lx >= -3072.0f ? 0.0f : lx + 3072.0f) : lx - 3072.0f;
            ly = ly <= 3072.0f ? (ly >= -3072.0f ? 0.0f : ly + 3072.0f) : ly - 3072.0f;

            strk = atan2f(ly, lx) * 65536.0f;
            
            if (camera_twp)
            {
                ang = -(GetCameraAngle(i)->y) - (strk * -0.1591549762031479);
            }
            else
            {
                ang = (strk * 0.1591549762031479);
            }

            float magnitude = ly * ly + lx * lx;
            strk = sqrtf(magnitude) * magnitude * 3.9187027e-14;
            if (strk > 1.0f)
            {
                strk = 1.0f;
            }
        }
        else
        {
            strk = 0.0f;
            ang = 0;
        }

        input_data[i] = { ang, strk };

        if (ucInputStatus == 1 && (i >= 4 || ucInputStatusForEachPlayer[i] == 1))
        {
            input_dataG[i] = input_data[i];
        }
        else
        {
            input_dataG[i] = { 0, 0.0f };
        }
	}
}


void InitPlayerPatches()
{
	PGetRotation_t = new Trampoline(0x44BB60, 0x44BB68, PGetRotation_r);
	WriteJump((void*)0x40F170, GetPlayersInputData_r);
}