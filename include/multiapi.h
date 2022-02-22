#ifndef _MULTIAPI_H
#define _MULTIAPI_H

#include <SADXStructs.h>
#include <SADXEnums.h>

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

	// Returns whether splitscreen is currently enabled
	API bool splitscreen_is_active(void);

	// Returns whether the given screen is enabled (ie. the player is connected)
	API bool viewport_is_enabled(int32_t num);

	// Saves the current viewport
	API void viewport_save(void);

	// Restores the saved viewport
	API void viewport_restore(void);

	// Change the viewport to draw sprites/models into another screenspace
	// Pass -1 to draw to whole screen
	API void viewport_change(int32_t num);

	// Returns the current screen that the game is drawing to
	API uint32_t viewport_get_num(void);

	// Get screen information as percentage of game screenspace.
	// ie. "w * HorizontalResolution" is the viewport width
	// Returns false if no information is avaiable
	API bool viewport_get_info(int32_t num, float* x, float* y, float* w, float* h);
	
	// Sets the character type of player pnum
	API void multi_set_charid(int32_t pnum, Characters character);

	// Reset all player scores to 0
	API void multi_score_reset(void);

	// Get the score of player pnum
	API int32_t multi_score_get(int32_t pnum);

	// Add to the score of player pnum
	API void multi_score_add(int32_t pnum, int32_t amount);

	// Reset all player lives to 4
	API void multi_lives_reset(void);

	// Get the amount of lives of player pnum
	API int32_t multi_lives_get(int32_t pnum);

	// Add lives to player pnum
	API void multi_lives_add(int32_t pnum, int32_t amount);

	// Reset rings of player pnum
	API void multi_rings_reset(int32_t pnum);

	// Get the amount of rings of player pnum
	API int32_t multi_rings_get(int32_t pnum);

	// Add rings to player pnum
	API void multi_rings_add(int32_t pnum, int32_t amount);

	// Returns true if multiplayer is currently enabled
	API bool multi_is_enabled(void);

	// Returns true if multiplayer is currently active (ingame)
	API bool multi_is_active(void);

	// Returns true if multiplayer is active and in battle mode
	API bool multi_is_battle(void);

	// Returns true if multiplayer is active and in coop mode
	API bool multi_is_coop(void);

	// Enable multiplayer mode
	API void multi_enable(int player_count, bool battle);

	// Disable multiplayer mode
	API void multi_disable(void);

	// Get the amount of connected players
	API uint32_t multi_get_player_count(void);

	// Get the ID of the winning player (-1 if none)
	API int32_t multi_get_winner(void);

	// Set the ID of the winning player (-1 to reset)
	API void multi_set_winner(int32_t pnum);

	// Calculate projection matrix of a player's camera
	API void camera_apply(uint32_t num);

	// Get camera position in `pos`, returns false if the camera does not exist
	API bool camera_get_pos(uint32_t num, NJS_POINT3* pos);

	// Get camera angle in `ang`, returns false if the camera does not exist
	API bool camera_get_ang(uint32_t num, Angle3* ang);

	// Set the position of a player's camera
	API void camera_set_pos(uint32_t num, float x, float y, float z);

	// Set the angle of a player's camera
	API void camera_set_ang(uint32_t num, Angle x, Angle y, Angle z);

	// Get the field of view of a player's camera
	API uint32_t camera_get_fov(uint32_t num);

	// Set the field of view of a player's camera
	// Default is `0x31C7` or `NJM_DEG_ANG(70.0f)`
	API void camera_set_fov(uint32_t num, Angle fov);

#ifdef __cplusplus
}
#endif

#undef API

#endif