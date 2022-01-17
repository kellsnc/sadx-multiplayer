#include "pch.h"
#include "multiplayer.h"

namespace multiplayer
{
    bool enabled = false;
    unsigned int pcount = 0;
    mode gMode = mode::coop;

    unsigned int GetPlayerCount()
    {
        return pcount;
    }

    void Enable(int player_count, mode md)
    {
        if (player_count > 0 && player_count <= PLAYER_MAX)
        {
            gMode = md;
            enabled = true;
            pcount = player_count;
        }
    }

    void Disable()
    {
        enabled = false;
        pcount = 0;
        ResetCharactersArray();
    }

    bool IsEnabled()
    {
        return enabled;
    }

    bool IsActive()
    {
        return IsEnabled() && (GameMode != GameModes_Menu) && pcount > 1;
    }

    bool IsBattleMode()
    {
        return IsActive() && gMode == mode::battle;
    }

    bool IsCoopMode()
    {
        return IsActive() && gMode == mode::coop;
    }

    bool IsFightMode()
    {
        return IsBattleMode() && CurrentLevel >= LevelIDs_Chaos0 && CurrentLevel <= LevelIDs_E101R;
    }
}