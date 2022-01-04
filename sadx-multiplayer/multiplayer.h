#pragma once

namespace multiplayer
{
    unsigned int GetPlayerCount();

    void Enable(int player_count);
    void Disable();

    bool IsEnabled();
    bool IsActive();
}