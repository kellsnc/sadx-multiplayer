#ifndef _MULTIAPI_H
#define _MULTIAPI_H

#include "ninja.h"
#include "SADXStructs.h"
#include "SADXStructsNew.h"
#include "SADXEnums.h"

#ifdef __cplusplus
#include <cstddef>
#include <cstdint>
#else
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#endif

#ifdef MULTI_API
#define API __declspec(dllexport)
#else
#define API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C"
{
#endif

	// Returns whether split screen is currently active (in game)
	// If it retuns true, at least two screens are currently rendering
	API bool splitscreen_is_active(void);

	// Returns whether split screen is enabled but not necessarily active
	// For example, split screen is inactive during events, menus, etc.
	API bool splitscreen_is_enabled();

	// Enable split screen
	// This is done automatically when multiplayer is enabled
	API void splitscreen_enable();

	// Disable split screen
	// This is done automatically when multiplayer is disabled
	API void splitscreen_disable();

	// Returns whether the given viewport is enabled
	API bool viewport_is_enabled(int32_t num);

	// Set the viewport that the game is rendering to
	// Pass -1 to draw to the whole screen
	API void viewport_set_num(int32_t num);

	// Get the viewport that the game is rendering to
	// Returns -1 if it is drawing to the whole screen
	API int32_t viewport_get_num(void);

	// Get screen information as percentage of game screen space.
	// ie. "w * HorizontalResolution" is the viewport width
	// Returns false if no information is avaiable
	API bool viewport_get_info(int32_t num, float* x, float* y, float* w, float* h);

	// Sets the character type of player pnum
	API void multi_set_charid(uint32_t pnum, Characters character);

	// Reset all player scores to 0
	API void multi_score_reset(void);

	// Get the score of player pnum
	API int32_t multi_score_get(uint32_t pnum);

	// Add to the score of player pnum
	API void multi_score_add(uint32_t pnum, int32_t amount);

	// Reset all player lives to 4
	API void multi_lives_reset(void);

	// Get the amount of lives of player pnum
	API int32_t multi_lives_get(uint32_t pnum);

	// Add lives to player pnum
	API void multi_lives_add(uint32_t pnum, int32_t amount);

	// Reset rings of player pnum
	API void multi_rings_reset(uint32_t pnum);

	// Get the amount of rings of player pnum
	API int32_t multi_rings_get(uint32_t pnum);

	// Add rings to player pnum
	API void multi_rings_add(uint32_t pnum, int32_t amount);

	// Returns true if multiplayer is currently enabled
	API bool multi_is_enabled(void);

	// Returns true if multiplayer is currently active (ingame)
	API bool multi_is_active(void);

	// Returns true if multiplayer is active and in battle mode
	API bool multi_is_battle(void);

	// Returns true if multiplayer is active and in coop mode
	API bool multi_is_coop(void);

	// Enable multiplayer mode, returns true on success
	API bool multi_enable(uint32_t player_count, bool battle);

	// Disable multiplayer mode, returns true on success
	API bool multi_disable(void);

	// Get the amount of connected players
	API uint32_t multi_get_player_count(void);

	// Get the ID of the winning player (-1 if none)
	API int32_t multi_get_winner(void);

	// Set the ID of the winning player (-1 to reset)
	API void multi_set_winner(uint32_t pnum);

	// Retrieve local (per-player) gravity of the given player
	// Returns false if the player is using the global gravity (pointers are not written to)
	API bool multi_get_gravity(uint32_t pnum, NJS_POINT3* v, Angle* angx, Angle* angz);

	// Set local (per-player) gravity for the given player
	// Once set, the player will ignore the global gravity
	API void multi_set_gravity(uint32_t pnum, Angle angx, Angle angz);

	// Disables local gravity for a given player
	API void multi_reset_gravity(uint32_t pnum);

	// Retrieve local (per-player) fog of the given player
	// Returns false if the player is using the global fog
	API bool multi_get_fog(uint32_t pnum, ___stcFog* pFog);

	// Set local (per-player) fog for the given player
	// Once set, the player will ignore the global fog
	API void multi_set_fog(uint32_t pnum, ___stcFog* pFog);

	// Disables local fog for a given player
	API void multi_reset_fog(uint32_t pnum);

	// Calculate projection matrix of a player's camera
	API void camera_apply(uint32_t num);

	// Get current camera position, returns a pointer that can be written to as well
	API NJS_POINT3* camera_get_pos(uint32_t num);

	// Get current camera angle, returns a pointer that can be written to as well
	API Angle3* camera_get_ang(uint32_t num);

	// Set the position of a player's camera
	API void camera_set_pos(uint32_t num, float x, float y, float z);

	// Set the angle of a player's camera
	API void camera_set_ang(uint32_t num, Angle x, Angle y, Angle z);

	// Get the field of view of a multiplayer camera
	API uint32_t camera_get_fov(uint32_t num);

	// Set the field of view of a multiplayer camera
	// Default is `0x31C7` or `NJM_DEG_ANG(70.0f)`
	API void camera_set_fov(uint32_t num, Angle fov);

	// Enable/disable free camera for a specific player
	// Note that the vanilla function `SetFreeCamera` will work for P1
	API void camera_enable_freecam(uint32_t pnum, bool enable);

	// Get if free camera is enabled for a specific player
	// Note that the vanilla function `GetFreeCamera` will work for P1
	API bool camera_is_freecam_enabled(uint32_t pnum);

	// Allow/disallow free camera for a specific player
	// Note that the vanilla function `SetFreeCameraMode` will work for P1
	API void camera_allow_freecam(uint32_t pnum, bool allow);

	// Get if free camera is allowed for a specific player
	// Note that the vanilla function `GetFreeCameraMode` will work for P1
	API bool camera_is_freecam_allowed(uint32_t pnum);

	// Override the current layout auto camera with another, see CAMMD and CAMADJ enums
	// Note that the vanilla function `CameraSetNormalCamera` will work for P1
	API void camera_set_normal_camera(uint32_t pnum, uint32_t ssCameraMode, uint32_t ucAdjustType);

	// Set an event camera, see CAMMD and CAMADJ enums
	// Note that the vanilla function `CameraSetEventCamera` will work for P1
	API void camera_set_event_camera(uint32_t pnum, uint32_t ssCameraMode, uint32_t ucAdjustType);

	// Set an event camera with a custom logic function, see CAMADJ and CDM enums
	// Note that the vanilla function `CameraSetEventCamera` will work for P1
	API void camera_set_event_camera_func(uint32_t pnum, CamFuncPtr fnCamera, uint32_t ucAdjustType, uint32_t scCameraDirect);

	// Release any active event camera and restore regular camera
	// Note that the vanilla function `CameraReleaseEventCmaera` will work for P1
	API void camera_release_event_camera(uint32_t pnum);

	// Get the target list and count for a specific player
	// Return false on error (pointers no written to)
	API bool multi_get_enemy_list(uint32_t pnum, colaround** pp_ael, Uint16* p_num);

#ifdef __cplusplus
}
#endif

#undef API

#endif