#include "pch.h"
#include <camera.h>

FunctionPointer(void, SetChaos4ColliParam, (taskwk* a1, int mode), 0x551840);

FastUsercallHookPtr<void(*)(taskwk* twp), noret, rEAX> chaos4ChkDmg_h(0x551F60);
void chaos4ChkDmg(taskwk* twp)
{
    if (multiplayer::IsActive() == false)
    {
        return chaos4ChkDmg_h.Original(twp);
    }

    __int16 flag; 
    static NJS_POINT3 v_3 = { -1.2f, 1.2f, 0.0f };
    flag = twp->flag;

    if ((flag & 4) != 0)
    {
        twp->flag = flag & 0xFFFB;
        if (twp->mode != 9)
        {
            chaos_reqmode = 9;
            chaos_oldmode = twp->mode;
        }
        twp->smode = -1;
        SetChaos4ColliParam(twp, 0);
        dsPlay_oneshot(363, 0, 0, 0);
        auto player = CCL_IsHitPlayer(twp);
        if (player)
        {
            auto pnum = player->counter.b[0];
            VibShot(pnum, 1);
            PConvertVector_P2G(playertwp[pnum], &v_3);
            SetVelocityP(pnum, v_3.x, v_3.y, v_3.z);
        }
    }
}

//chaos 4 is one of the rare object of the game that does not affect the display function to the task
//this prevents splitscreen to draw chaos for others players, we patch this by creating a dummy display function that call the original one.
void Chaos4_Display_r(task* tp)
{
    Chaos4_Display(tp->twp, (chaoswk*)tp->awp);
}

FastFunctionHook<void, task*> Chaos4_Exec_h(0x552960);
void Chaos4_Exec_r(task* tp)
{
   
    auto wk = (chaoswk*)tp->awp;

    Chaos4_Exec_h.Original(tp);
    tp->disp(tp);

}

task* setChaos4_r()
{
    auto tp = CreateElementalTask(LoadObj_Data1 | LoadObj_Data2, 1, TaskFuncPtr(0x552960));
    tp->disp = Chaos4_Display_r; //assign the task as god intended
    return tp;
}

void patch_chaos4_init()
{
    chaos4ChkDmg_h.Hook(chaos4ChkDmg);
    WriteData<5>((int*)0x552B78, 0x90);
    WriteJump((void*)0x552BB0, setChaos4_r);
    Chaos4_Exec_h.Hook(Chaos4_Exec_r);
}

#ifdef MULTI_TEST
    RegisterPatch patch_chaos4(patch_chaos4_init);
#endif