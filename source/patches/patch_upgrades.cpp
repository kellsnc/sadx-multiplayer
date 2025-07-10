#include "pch.h"

FastUsercallHookPtr<int(*)(task* tp, taskwk* twp, int a3), rEAX, rEAX, rEBP, rEBX> levelupitemEffectManager_h(0x4BF320);

struct LEV_TIKAL
{
    int mode;
    int counter;
    task* tp;
    NJS_POINT3 pos;
    float scl;
    float alpha;
};

struct LEV_TUBU
{
    Angle3 ang;
    Angle3 spd;
    float anim;
    float anim_spd;
};

enum LEVUP_STAGE_NO : __int32
{
    SS = 0x0,
    MR = 0x1,
    EC = 0x2,
    ANY = 0x3,
};


const struct LevelItemAccess_ //guessed name
{
    __int16 player;
    LEVUP_STAGE_NO stage;
};

struct OBJECT_LEVELUPITEM_DATA
{
    int flag;
    int item;
    int counter;
    float motion_frame[3];
    float alpha[3];
    NJS_POINT3 item_pos;
    Angle3 item_ang;
    LEV_TIKAL tikal;
    LEV_TUBU tubu[4];
};


DataPointer(OBJECT_LEVELUPITEM_DATA*, levelupitem_data, 0x03C5A938);
DataArray(LevelItemAccess, appearStage, 0x7E6798, 18);

signed int levelupitemEffectManager(task* tp, taskwk* twp, int a3)
{
    if (multiplayer::IsActive())
    {
        auto result = levelupitem_data->counter;
        if (result)
        {
            if (result >= 88)
            {
                WriteData<3>((int*)0x4BF3B7, 0x90);

                if (levelupitem_data->item < 12)
                {
                    for (uint8_t i = 0; i < multiplayer::GetPlayerCount(); i++)
                    {

                        auto p = playertwp[i];

                        if (!p)
                            continue;

                        if (p->counter.b[1] == appearStage.at((int)twp->scl.x).Character)
                        {
                            playerpwp[i]->equipment |= 1 << levelupitem_data->item;
                        }
                    }
                }
            }

        }
    }
    else
    {
        auto result = levelupitem_data->counter;
        if (result)
        {
            if (result >= 88)
            {
                WriteData<1>((int*)0x4BF3B7, 0x83);
                WriteData<1>((int*)0x4BF3B8, 0xC0);
                WriteData<1>((int*)0x4BF3B9, 0x4);
            }
        }
    }

	return levelupitemEffectManager_h.Original(tp, twp, a3);
}

void patch_upgrades_init()
{
    levelupitemEffectManager_h.Hook(levelupitemEffectManager);
}

RegisterPatch patch_upgrades(patch_upgrades_init);