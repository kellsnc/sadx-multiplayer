#pragma once

namespace multiplayer
{
    enum class mode
    {
        coop,
        battle
    };

    mode GetMode();
    unsigned int GetPlayerCount();

    void Enable(int player_count, mode md);
    void Disable();

    bool IsEnabled();
    bool IsActive();
}