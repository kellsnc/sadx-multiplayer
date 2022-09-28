![Documentation logo consisting of the original game logo and a "Multiplayer" text next to it](./images/banner-logo.png)

# Using the API

The Multiplayer mod exports a handful of functions to allow mod developpers to access and modify useful data. With these, you can access the rings of other players, the current screen being drawn to, etc.

There are two methods to get these exported functions: linking the import library (recommended) and manually retrieving the function pointers.

## Linking the import library (recommended)

You will find the import library in every release package at `api\sadx-multiplayer.lib` and the function definitions at `api\multiapi.h`. Move these files into your own project.

Then open the project settings and add `sadx-multiplayer.lib` to your additional libraries. In the screenshot below, the lib was placed at the solution's root, you can always change the relative path.

Then in "Delay Loaded Dlls", add `sadx-multiplayer.dll` (if you don't, people will need your mod to be loaded after the Multiplayer mod.)

![Image showing the way to add a library to the "additional dependencies" list](./images/linking-library.png)

Now if you include `multiapi.h` in a cpp file you can call the functions provided by the API.

You may want to add a boolean to check if the multiplayer mod is loaded, otherwise your mod will crash when trying to call an API function.

Somewhere outside of a function:
```
bool gMultiplayerModLoaded = false;
```

In your init function:
```
gMultiplayerModLoaded = GetModuleHandle(L"sadx-multiplayer.dll") != NULL;
```

Usage exemple:
```
void AddScore(uint32_t pnum, int32_t amount)
{
    if (gMultiplayerModLoaded)
    {
        multi_score_add(pnum, amount);
    }
    else // original behaviour if the mod is not loaded
    {
        if (pnum == 0)
        {
            EnemyBonus += amount;
        }
    }
}
```

Now whenever the multiplayer mod is updated with new API function, you just have to edit the `sadx-multiplayer.lib` and `multiapi.h` and the new functions will be available to you!

## Manual method

The downside of this method is that you have to provide the function definition yourself which can be tedious.

First, you need to get a handle to the sadx-multiplayer DLL.

```
HMODULE handle = GetModuleHandle(L"sadx-multiplayer.dll");
```

Then you can look for the function using `GetProcAddress`

```
if (handle)
{
    auto multi_score_add = GetProcAddress(handle, "multi_score_add");
}
```

It's not done yet, you cannot call "multi_score_add" because your code doesn't know it's a function. We need to tell it its type.

```
auto multi_score_add = (void(*)(uint32_t pnum, int32_t amount))GetProcAddress(handle, "multi_score_add");
```

Doing this every time you want to call a function is both slow and tedious, so I recommend using global variable.

```
void(*multi_score_add)(uint32_t pnum, int32_t amount) = nullptr;

void InitializeMultiplayerAPI()
{
	HMODULE handle = GetModuleHandle(L"sadx-multiplayer.dll");

	if (handle)
	{
		multi_score_add = (void(*)(uint32_t pnum, int32_t amount))GetProcAddress(handle, "multi_score_add");

		// ...
	}
}

void AddScore(uint32_t pnum, int32_t amount)
{
	if (multi_score_add)
	{
		multi_score_add(pnum, amount);
	}
	else // original behaviour if the mod is not loaded
	{
		if (pnum == 0)
		{
			EnemyBonus += amount;
		}
	}
}

```

With this method, you need to add a function pointer for every function you want and get their address manually.

## Functions

### Gameplay

**multi_score_reset**
```
void multi_score_reset(void);
```
Reset the score counter of every player.

**multi_score_get**
```
int32_t multi_score_get(uint32_t pnum);
```
Get the current score of a specific player. Returns 0 for non-valid player IDs.

**multi_score_add**
```
void multi_score_add(uint32_t pnum, int32_t amount);
```
Add to the score of a specific player. No behaviour if you exceed the number of active players.

**multi_lives_reset**
```
void multi_lives_reset(void);
```
Reset the life counter of every player.

**multi_lives_get**
```
int32_t multi_lives_get(uint32_t pnum);
```
Get the current amount of lives of a specific player. Returns 0 for non-valid player IDs.

**multi_lives_add**
```
void multi_lives_add(uint32_t pnum, int32_t amount);
```
Add or remove lives for a specific player. No behaviour if you exceed the number of active players.

**multi_rings_reset**
```
void multi_rings_reset(void);
```
Reset the ring counter of every player.

**multi_rings_get**
```
int32_t multi_lives_get(uint32_t pnum);
```
Get the current amount of rings of a specific player. Returns 0 for non-valid player IDs.

**multi_rings_add**
```
void multi_rings_add(uint32_t pnum, int32_t amount);
```
Add or remove rings for a specific player. No behaviour if you exceed the number of active players.

**multi_get_winner**
```
int32_t multi_get_winner();
```
Get the player currently declared as the winner. This is only definitive once the result screen has appeared (battle mode.)

**multi_set_winner**
```
void multi_set_winner(uint32_t pnum);
```
Set the player that will be declared as the winner when the result screen appears (battle mode.)

### System

**multi_is_enabled**
```
bool multi_is_enabled();
```
If multiplayer mod is enabled.

**multi_is_active**
```
bool multi_is_active();
```
If multiplayer mod is active (in game)

**multi_is_battle**
```
bool multi_is_battle();
```
Returns true if the current multiplayer mode is battle.

**multi_is_coop**
```
bool multi_is_coop();
```
Returns true if the current multiplayer mode is cooperation.

**multi_set_charid**
```
void multi_set_charid(uint32_t pnum, Characters character);
```
Set the character ID for a specific player. `-1` means the character should not spawn. Otherwise use the Characters enum provided by the Mod Loader.

**multi_enable**
```
void multi_enable(uint32_t player_count, bool battle);
```
Enable multiplayer mode. Other players will be loaded once ingame if they don't exist yet and have a character ID. To set one, use multi_set_charid.

**multi_disable**
```
void multi_disable();
```
Disable multiplayer mode, does not remove any loaded player.

**multi_get_player_count**
```
uint32_t multi_get_player_count();
```
Get the amount of multiplayer players (and not the number of loaded characters.)

### Camera

The camera system was entirely rewritten to support other screens. As such, I provide a multiplayer counterpart for most of the original game functions.

**camera_apply**
```
void camera_apply(uint32_t num);
```
Applies a specific camera view (projection, fov, etc.) to the current screen. You will probably never have to use this!

**camera_get_pos**
```
bool camera_get_pos(uint32_t num, NJS_POINT3* pos);
```
Returns a pointer to a specific camera position vector. All cameras are always accessible even if the player doesn't exist, so it will only return nullptr if the ID goes beyond 4.

**camera_get_ang**
```
Angle3* camera_get_ang(uint32_t num);
```
Returns a pointer to a specific camera rotation struct. All cameras are always accessible even if the player doesn't exist, so it will only return nullptr if the ID goes beyond 4.

**camera_set_pos**
```
void camera_set_pos(uint32_t num, float x, float y, float z);
```
Change the current position of a specific camera. No behaviour if the ID is beyond 4. Note that the camera position will most likely be overwritten by the current auto camera.

**camera_set_ang**
```
void camera_set_ang(uint32_t num, Angle x, Angle y, Angle z);
```
Change the current rotation of a specific camera. No behaviour if the ID is beyond 4. Note that the camera position will most likely be overwritten by the current auto camera.

**camera_get_fov**
```
uint32_t camera_get_fov(uint32_t num);
```
Get the field of view of a specific camera in Ninja Angle (0-0x10000).

**camera_set_fov**
```
void camera_set_fov(uint32_t num, Angle fov);
```
Set the field of view of a specific camera, it takes a Ninja angle (0-0x10000.) To reset, pass angle 0.

**camera_enable_freecam**
```
void camera_enable_freecam(uint32_t pnum, bool enable);
```
Enable/disable free camera mode for a specific screen.

**camera_is_freecam_enabled**
```
bool camera_is_freecam_enabled(uint32_t pnum);
```
Returns true if free camera mode is enabled for a specific player.

**camera_allow_freecam**
```
void camera_allow_freecam(uint32_t pnum, bool allow);
```
Allow/forbid free camera mode for a specific player.

**camera_is_freecam_allowed**
```
bool camera_is_freecam_allowed(uint32_t pnum);
```
Returns true if free camera is allowed for a specific player.

**camera_set_normal_camera**
```
void camera_set_normal_camera(uint32_t pnum, uint32_t ssCameraMode, uint32_t ucAdjustType);
```
Set the current auto camera for a specific player, will be overwritten as soon as you reach a camera layout trigger.

**camera_set_event_camera**
```
void camera_set_event_camera(uint32_t pnum, uint32_t ssCameraMode, uint32_t ucAdjustType);
```
Sets an event camera for a specific player. Event cameras can only be reset manually, or overwritten by another one.

**camera_set_event_camera_func**
```
void camera_set_event_camera_func(uint32_t pnum, CamFuncPtr fnCamera, uint32_t ucAdjustType, uint32_t scCameraDirect);
```
Sets an event camera for a specific player with a custom logic function. Event cameras can only be reset manually, or overwritten by another one.

**camera_release_event_camera**
```
void camera_release_event_camera(uint32_t pnum);
```
Release an event camera for a specific player if one is running.

### Split-screen

**splitscreen_is_active**
```
bool splitscreen_is_active(void);
```
Returns true is split-screen is currently running.

**viewport_is_enabled**
```
bool viewport_is_enabled(int32_tnum);
```
If a specific split-screen exists on screen.

**viewport_save**
```
void viewport_save(void);
```
Save the current screen ID.

**viewport_restore**
```
void viewport_restore(void);
```
Restore the last saved screen ID.

**viewport_change**
```
void viewport_change(int32_t num);
```
Change the screen ID used for rendering. If you set this to `-1`, it will render to the whole screen.

Use `viewport_save` before and `viewport_restore` after to not break the rendering queue.

**viewport_get_num**
```
uint32_t viewport_get_num(void);
```
Get the current screen being drawn to. This only works in the display subroutines of tasks, otherwise it returns the last screen that was drawn to.

This is useful for rendering things only on specific screens.

**viewport_get_info**
```
bool viewport_get_info(int32_t num, float* x, float* y, float* w, float* h);
```
Get the information of a specific split-screen. Returns true if the operation succeeded and values were written to. False otherwise.
