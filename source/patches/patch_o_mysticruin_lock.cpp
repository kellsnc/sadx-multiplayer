#include "pch.h"

FastFunctionHook<int, int>GetLockOpen_h(0x53C630);
DataArray(int[8], MRLockSeqVarID, 0x111DA98, 4);

int __cdecl GetLockOpen_r(int kind)
{
    if (multiplayer::IsActive() == false)
    {
        return GetLockOpen_h.Original(kind);
    }


    int v1;
    int result = 0;

    v1 = MRLockSeqVarID[CurrentCharacter][kind];
    if (v1 < 0)
    {
        result = 0;
    }
    else
    {
        result = (unsigned __int8)seqVars[v1];
    }
    return result;
}


void patch_o_mysticruin_lock_init()
{
    GetLockOpen_h.Hook(GetLockOpen_r);
}

RegisterPatch patch_o_mysticruin_lock(patch_o_mysticruin_lock_init);