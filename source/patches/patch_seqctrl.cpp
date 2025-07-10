#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include <camera.h>



// Change character boss ID from 1 to 7 to avoid conflict with other players.

FastFunctionHook<void, int> SeqExecBossPlayer_h(0x413840);
FastFunctionHook<int, task*> EventBossCameraSet_h(0x4C2C60);
FastFunctionHook<void> EventBossCameraSetFunc_h(0x4C2C40);


static uint8_t aiPAD = 7;

VariableHook<EVBOSS_CAMERA, 0x3C5B260> EventBossCameraWork_m;
DataPointer(Float, fOffset_0, 0x981DCC);

void __cdecl EventBossCamera()
{
    taskwk* p1; 
    taskwk* p2;
    playerwk* pwp; 
    Float v3; 
    Float v4; 
    Float v5; 
    Float v6; 
    int v7; 
    int v8; 
    NJS_VECTOR a1; 

    auto pnum = TASKWK_PLAYERID(playertwp[0]);

    if (camera_twp)
    {
        if (&cameraControlWork)
        {
            p1 = playertwp[pnum];
            p2 = playertwp[aiPAD];
            EventBossCameraWork_m[pnum].ptwp = playertwp[pnum];
            EventBossCameraWork_m[pnum].btwp = playertwp[aiPAD];
            pwp = playerpwp[pnum];
            if (p1)
            {
                if (p2 && pwp)
                {
                    if (EventBossCameraWork_m[pnum].mode)
                    {
                        if (EventBossCameraWork_m[pnum].mode == 1)
                        {
                            v4 = cameraControlWork.tgtdist * 0.060f + pwp->p.eyes_height;
                            EventBossCameraWork_m[pnum].offset.z = fOffset_0;
                            EventBossCameraWork_m[pnum].offset.y = v4;
                            a1.x = p1->pos.x - p2->pos.x;
                            a1.y = p1->pos.y - p2->pos.y;
                            a1.z = p1->pos.z - p2->pos.z;
                            if (njScalor_copy(&a1) != 0.0f)
                            {
                                ghUnitVectorXZ(&a1, &a1);
                                v5 = EventBossCameraWork_m[pnum].offset.z * a1.x;
                                EventBossCameraWork_m[pnum].pos.x = v5 + p1->pos.x;
                                EventBossCameraWork_m[pnum].pos.y = EventBossCameraWork_m[pnum].offset.y + p1->pos.y;
                                v6 = EventBossCameraWork_m[pnum].offset.z * a1.z;
                                EventBossCameraWork_m[pnum].pos.z = v6 + p1->pos.z;
                                EventBossCameraWork_m[pnum].inter.x = p2->pos.x - v5;
                                EventBossCameraWork_m[pnum].inter.y = EventBossCameraWork_m[pnum].pos.y - EventBossCameraWork_m[pnum].offset.y;
                                EventBossCameraWork_m[pnum].inter.z = p2->pos.z - v6;
                            }
                            CameraSmooth(p1, &EventBossCameraWork_m[pnum], p2);
                        }
                        else if (EventBossCameraWork_m[pnum].mode == 2)
                        {
                            if (EventBossCameraWork_m[pnum].ptwp->counter.b[1] == 6)
                            {
                                v3 = cameraControlWork.tgtdist * 0.060f + pwp->p.eyes_height + 10.0f;
                            }
                            else
                            {
                                v3 = pwp->p.eyes_height * 3.0f;
                            }
                            EventBossCameraWork_m[pnum].offset.y = v3;
                            EventBossCameraWork_m[pnum].offset.z = fOffset_0;
                            a1.x = p1->pos.x - p2->pos.x;
                            a1.y = p1->pos.y - p2->pos.y;
                            a1.z = p1->pos.z - p2->pos.z;
                            ghUnitVectorXZ(&a1, &a1);
                            EventBossCameraWork_m[pnum].pos.x = EventBossCameraWork_m[pnum].offset.z * a1.x + p1->pos.x;
                            EventBossCameraWork_m[pnum].pos.y = EventBossCameraWork_m[pnum].offset.y + p1->pos.y;
                            EventBossCameraWork_m[pnum].pos.z = EventBossCameraWork_m[pnum].offset.z * a1.z + p1->pos.z;
                            EventBossCameraWork_m[pnum].inter.x = (p2->pos.x + p1->pos.x) * 0.5f;
                            EventBossCameraWork_m[pnum].inter.y = (p1->pos.y + p2->pos.y) * 0.5f;
                            EventBossCameraWork_m[pnum].inter.z = (p2->pos.z + p1->pos.z) * 0.5f;
                            CameraSmooth(p1, &EventBossCameraWork_m[pnum], p2);
                        }
                        goto LABEL_28;
                    }
                    EventBossCameraWork_m[pnum].offset.x = 0.0f;
                    EventBossCameraWork_m[pnum].offset.y = cameraControlWork.tgtdist * 0.060f + pwp->p.eyes_height;
                    njPushMatrix(nj_unit_matrix_);
                    if (EventBossCameraWork_m[pnum].ang.y)
                    {
                        njRotateY(0, LOWORD(EventBossCameraWork_m[pnum].ang.y));
                    }
                    njRotateX(0, 63488);
                    EventBossCameraWork_m[pnum].offset.z = fOffset_0;
                    njCalcPoint(0, &EventBossCameraWork_m[pnum].offset, &EventBossCameraWork_m[pnum].pos);
                    njAddVector(&EventBossCameraWork_m[pnum].pos, &p1->pos);

                    EventBossCameraWork_m[pnum].offset.z = -fOffset_0;
                    njCalcPoint(0, &EventBossCameraWork_m[pnum].offset, &EventBossCameraWork_m[pnum].inter);
                    EventBossCameraWork_m[pnum].inter.x = EventBossCameraWork_m[pnum].inter.x + p1->pos.x;
                    EventBossCameraWork_m[pnum].inter.y = EventBossCameraWork_m[pnum].inter.y + p1->pos.y;
                    EventBossCameraWork_m[pnum].inter.z = EventBossCameraWork_m[pnum].inter.z + p1->pos.z;
                    njPopMatrix(1u);
                    v7 = per[pnum]->on;
                    if ((v7 & 0x200) != 0)
                    {
                        if ((v7 & 0x20000) != 0)
                        {
                            fOffset_0 = fOffset_0 + 2.0f;
                        }
                        else if ((v7 & 0x10000) != 0)
                        {
                            fOffset_0 = fOffset_0 - 2.0f;
                        }
                        goto LABEL_28;
                    }
                    if ((v7 & 0x20000) != 0)
                    {
                        v8 = EventBossCameraWork_m[pnum].ang.y - 512;
                    }
                    else
                    {
                        if ((v7 & 0x10000) == 0)
                        {
                        LABEL_28:
                            if ((((unsigned __int16)ssActNumber | (unsigned __int16)(ssStageNumber << 8)) & 0xFF00) == 7424)
                            {
                                if (EventBossCameraWork_m[pnum].pos.z <= 3560.0f)
                                {
                                    if (EventBossCameraWork_m[pnum].pos.z < 3180.0f)
                                    {
                                        EventBossCameraWork_m[pnum].pos.z = 3180.0f;
                                    }
                                }
                                else
                                {
                                    EventBossCameraWork_m[pnum].pos.z = 3560.0f;
                                }
                            }
                            cameraControlWork.tgtxpos = EventBossCameraWork_m[pnum].inter.x;
                            cameraControlWork.tgtypos = EventBossCameraWork_m[pnum].inter.y;
                            cameraControlWork.tgtzpos = EventBossCameraWork_m[pnum].inter.z;
                            cameraControlWork.camxpos = EventBossCameraWork_m[pnum].pos.x;
                            cameraControlWork.camypos = EventBossCameraWork_m[pnum].pos.y;
                            cameraControlWork.camzpos = EventBossCameraWork_m[pnum].pos.z;
                            cameraControlWork.angz = 0;
                            cameraControlWork.angy = 0;
                            cameraControlWork.angx = 0;
                            return;
                        }
                        v8 = EventBossCameraWork_m[pnum].ang.y + 512;
                    }
                    EventBossCameraWork_m[pnum].ang.y = v8;
                    goto LABEL_28;
                }
            }
        }
    }
}

int EventBossCameraSet_r(task* tp)
{

    if (multiplayer::IsActive() == false)
    {
        return EventBossCameraSet_h.Original(tp);
    }

    anywk* v1; 
    playerwk* pwp;
    int result; 
    v1 = tp->awp;

    for (uint8_t i = 0; i < multiplayer::GetPlayerCount(); i++)
    {
        CameraSetEventCameraFunc_m(i, (void(__cdecl*)(_OBJ_CAMERAPARAM*))EventBossCamera, 0, 2);
        memset(&EventBossCameraWork_m, 0, sizeof(EventBossCameraWork_m));
        EventBossCameraWork_m[i].mode = 2;
        EventBossCameraWork_m[i].bctp = tp;
        EventBossCameraWork_m[i].maxw.x = 256;
        EventBossCameraWork_m[i].maxw.y = 256;
        pwp = playerpwp[i];
        if (pwp)
        {
            
            EventBossCameraWork_m[i].offset.x = 0.0f;
            EventBossCameraWork_m[i].offset.z = 0.0f;
            EventBossCameraWork_m[i].offset.y = cameraControlWork.tgtdist * 0.060f + pwp->p.eyes_height;
        }
        else
        {
            EventBossCameraWork_m[i].offset.y = 50.0f;
            EventBossCameraWork_m[i].offset.z = 50.0f;
        }
    }

    result = v1->anywk::work.sb[2];
    switch (v1->anywk::work.ub[2])
    {
    case 0u:
    case 2u:
    case 3u:
        fOffset_0 = 50.0f;
        break;
    case 1u:
    case 4u:
    case 5u:
        fOffset_0 = 70.0f;
        break;
    default:
        return result;
    }
    return result;
}

void EventBossCameraSetFunc_r()
{
    if (multiplayer::IsActive())
    {
        for (uint8_t i = 0; i < multiplayer::GetPlayerCount(); i++)
        {
            CameraSetEventCameraFunc_m(i, (void(__cdecl*)(_OBJ_CAMERAPARAM*))EventBossCamera, 0, CDM_LOOKAT);
        }
    }
    else
    {
        EventBossCameraSetFunc_h.Original();
    }
}


void SetNPCController()
{
	aiPAD = multiplayer::IsActive() ? 7 : 1;
	
	//Knux AI
	WriteData<1>((uint8_t*)0x4D5E3A, aiPAD);
	WriteData((int*)0x4D5E42, (int)&perG[aiPAD].on);
	WriteData((int*)0x4D5E48, (int)&perG[aiPAD].press);


	WriteData((int*)0x4D64EE, (int)&input_dataG[aiPAD].angle);
	WriteData((int*)0x4D64F4, (int)&input_dataG[aiPAD].stroke);

	WriteData((int*)0x4D64FA, (int)&perG[aiPAD].on);
	WriteData((int*)0x4D6500, (int)&perG[aiPAD].press);
	WriteData((int*)0x4D644A, (int)&perG[aiPAD].y1);

	//Sonic AI
    WriteData<1>((uint8_t*)0x4B6464, aiPAD);
	WriteData((int*)0x4B646C, (int)&perG[aiPAD].on);
	WriteData((int*)0x4B6472, (int)&perG[aiPAD].press);

    WriteData((int*)0x4B6C92, (int)&perG[aiPAD].on);
    WriteData((int*)0x4B6C98, (int)&perG[aiPAD].press);


    //set AI to player 7
	WriteData<1>((uint8_t*)0x4B6089, aiPAD);
	WriteData<1>((uint8_t*)0x4B60AD, aiPAD); 
	WriteData<1>((uint8_t*)0x4B60D1, aiPAD);

    //get status stuff
    WriteData<1>((uint8_t*)0x4D6406, aiPAD);
    WriteData<1>((uint8_t*)0x4D642F, aiPAD);
    WriteData<1>((uint8_t*)0x4D644F, aiPAD);

}

void __cdecl SeqExecBossPlayer_r(int boss_id)
{
	SetNPCController();
	SeqExecBossPlayer_h.Original(boss_id);
}

void patch_seqctrl_init()
{
	SeqExecBossPlayer_h.Hook(SeqExecBossPlayer_r);
    EventBossCameraSet_h.Hook(EventBossCameraSet_r);
    EventBossCameraSetFunc_h.Hook(EventBossCameraSetFunc_r);
}

RegisterPatch patch_seqctrl(patch_seqctrl_init);