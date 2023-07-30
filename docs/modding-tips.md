![Documentation logo consisting of the original game logo and a "Multiplayer" text next to it](./images/banner-logo.png)

# Modding with multiplayer in mind

Mod compatiblity is our top priority, we did everything we could to ensure compatiblity with existing mods. Some mods however may only be partially compatible if the code was not made with multiplayer in mind. To ensure that your mod is fully compatible with multiplayer, here is a list of best practices.

## Getting information from the multiplayer mod

You can access basic information using the Mod Loader API.

```
auto multi_mod = helperFunctions.Mods->find("sadx-multiplayer"); // Requires helperFunctions.Version >= 16
if (multi_mod) // Check if the mod is loaded
{
    // You can now access information like the mod version, its path, etc.
}
```

Now if you want access to more interesting things (like P2+ rings, lives, score, camera, etc.) you will need to use the API that the mod provides.

You can learn about the Multiplayer API [here](./using-the-api.md).

## Working with every player

As the game is originally singleplayer only, some mods tend to only care for player 1. In fact, the game itself rarely even checks for more than the first two players. While we did patch most of the game, we cannot patch mods.

The game technically supports 8 simultaneous players, however **we chose to only support 4 players** for various reasons. You can either loop for the first 4 players or for the whole table.

Here's the most basic way to loop for players
```
for (int i = 0; i < 4; ++i) // or playertwp.size() to loop for the whole player table
{
    auto ptwp = playertwp[i];

    if (ptwp) // check if the player exists
    {
        ...
    }
}
```

The mod loader wraps the game arrays into a convenient class, so you can do range loops:
```
for (auto& ptwp : playertwp)
{
    if (ptwp)
    {
        ...
    }
}
```

## Using tasks/objects properly

### 1. Using the display function

Tasks (objects) contain 3 functions:
* `disp`: the function that contains the rendering code
* `exec`: the function that contains the logic code
* `dest`: a function called when the object is being deleted

It is important that you use the exec and display functions appropriately because the display function is actually called once per screen. So if you include logic in the display function, it will run up to 4 times faster than it should.

Contrary to SA2, SADX does not call the display function automatically (except when the game is paused). So you have to call the display function manually in the exec for the first screen. The multiplayer mod will call the display automatically for the other screens.

```
void __cdecl mytaskdisp(task* tp)
{
    if (!loop_count) // do not render if the game is catching up
    {

    }
}

void __cdecl mytask(task* tp)
{
    auto twp = tp->twp;
    if (twp->mode == 0)
    {
        tp->disp = mytaskdisp;
        twp->mode = 1;
    }
    
    tp->disp(tp);
}
```

Fun fact: the developpers sometimes forgot to set up the display function, you can try to pause the game to find all the places where they forgot it! We patched those cases in the mod.

### 2. Using forcewk

If you made a task/object with a model collision that moves, you may want players to move with it. The game internally uses forcewk, or fwp. However, they only set up 2 per tasks. When the multiplayer mod is active, the number of forcewk is bumped to 4. You must ensure yourself that the multiplayer mod is running (see the API), otherwise trying to access the extra fwp will be undefined behaviour.

Here's basic pseudocode:
```
void __cdecl mytask(task* tp)
{
    auto twp = tp->twp;
    if (twp->mode == 0)
    {
        // model collision setup
        tp->disp = mytaskdisp;
        tp->dest = mytaskdest; // destructor should remove the model collision
        twp->mode = 1;
    }

    twp->pos.x += 1.0f; // move 1 unit along the x-axis
    // update model collision here

    for (int i = 0; i < (multi_is_active() ? 4 : 2); ++i)
    {
        auto fwp = tp->fwp[i];
        fwp->pos_spd.x = 1.0f; // players touching the collision will move 1 unit along the x-axis
    }
    
    tp->disp(tp);
}
```

Note: this system is also used for conveyor belts for example.

## Misc

### Super Sonic

If you want to check if a specific player is Super Sonic, please use:

```
if (playerpwp[ID]->equipment & Upgrades_SuperSonic)
```

The multiplayer mod will set the global Super Sonic flag (`flagSuperSonicMode`) only for player 1.

### Metal Sonic flag

The Character Select mod allow individual players to be Metal Sonic. Unfortunately there is currently no way to easy way to check if a specific player is Metal Sonic. The global Metal Sonic flag (`gu8flgPlayingMetalSonic`) will be the state of the last loaded Sonic.
