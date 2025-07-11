#pragma once

typedef struct tornadewk
{
    Float ofsx;
    Float ofsy;
    Float ofsz;
    Float pathx;
    Float pathy;
    Float pathz;
    Float speed;
    Float Alpha;
    Sint32 rotx;
    Sint32 roty;
    Sint32 rotz;
    Sint32 hp;
    Sint32 score;
    Sint32 RapidFire;
    Sint32 RapidCnt;
    Sint32 LockTame;
} tornadewk;

enum
{
    MD_TORNADE_INIT = 0x0,
    MD_TORNADE_STND = 0x1,
    MD_TORNADE_DAMG = 0x2,
    MD_TORNADE_PLON = 0x3,
    MD_TORNADE_DEAD = 0x4,
    MD_TORNADE_DEAD2 = 0x5,
    MD_TORNADE_EVENT = 0x6,
    MD_TORNADE_EDIT = 0x7, //Unused
};

typedef struct plmodel
{
    NJS_TEXLIST* tex;
    NJS_ACTION* act;
} plmodel;

//custom
typedef struct CamBackWk
{
    Sint8 ccs_mode;
    Sint8 unk;
    Sint32 ShtDeadFrame; //unused
    Angle bakAngY;
    Float bakPosY;
    Float distanceTarget;
    Float dist;
    Sint32 unk1;
    NJS_POINT3 targetPos;
    taskwk* twp;
    Sint32 unk3;
    Float unk4;
    Angle3 ang;
    NJS_POINT3 Pos;
}CamBackWk;