#include "pch.h"
#include "multiplayer.h"

namespace multiplayer
{
    bool enabled = false;
    unsigned int pcount = 0;

    unsigned int GetPlayerCount()
    {
        return pcount;
    }

    void Enable(int player_count)
    {
        if (player_count > 0 && player_count <= PLAYER_MAX)
        {
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
}