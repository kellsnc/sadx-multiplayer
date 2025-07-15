#ifndef _CHARSAPI_H
#define _CHARSAPI_H

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

#ifdef CHARS_API
#define API __declspec(dllexport)
#else
#define API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C"
{
#endif


	/*
	* Callback used to run extra code in the display or exec function of a character, this can also override them
	* Run before the original function.
	* A pointer to the custom task function (display) which will act like a callback
	* characterIndex: The index of your character that you get when you register a character
	* shouldOverride: Specify if your custom task should override the original code from Character Importer, if unsure, let it to false.
	*/
	typedef void(__cdecl* char_importer_pre_load_cb)(task* tp);

	/*
	* Callback used to run extra code in the display or exec function of a character
	* Run AFTER the original function.
	* A pointer to the custom task function (display) which will act like a callback
	* characterIndex: The index of your character that you get when you register a character
	*/
	typedef void(__cdecl* char_importer_post_load_cb)(task* tp);

	/*@brief
	* Register a new skin to a character, press Y on the character on the CharSel menu to swap.
	* Return the id of the skin if it worked or -1 if it failed.
	* @param jsonPath: Specify the path of the json which contains the skin information.
	* @param modFolder: The path of your mod folder
	*/
	API int16_t Register_NewSkin(const char* jsonPath, const char* modFolder);
	/*@brief
	Register a new character based on an existing one. This is an advanced method that still require a lot of manual work after using this.
	* Return the id of the character if it worked or -1 if it failed.
	* @param jsonPath: Specify the path of the json which contains the new character information.
	* @param modFolder: The path of your mod folder
	*/

	API int16_t Register_NewCharacter(const char* jsonPath, const char* modFolder);

	/*@brief
	* Get an added character data with its ID
	* @param ID: The id that you get when Registering a Character.
	*/
	API NewChar* GetNewCharacterPerID(uint16_t ID);
	/*@brief
	* Get an added character data with its name.
	* @param name: The name that you used for the character in the json file.
	*/
	API NewChar* GetNewCharacterPerName(const std::string name);
	/*@brief
	* Register a custom task function for your character, useful if you want to add extra code like gameplay for your character
	* The callback run before the original function.
	* @param callback: A pointer to the custom task function (exec) which will act like a callback
	* @param characterIndex: The unique index of your character
	* @param shouldOverride: Specify if your custom task should override the original code from Character Importer, if unsure, let it to false.
	*/
	API void register_task_pre_exec(char_importer_pre_load_cb callback, uint16_t characterIndex, bool shouldOverride);
	/*@brief
	* Register a custom task function for your character, useful if you want to add extra code like specific effect or animation in the display function.
	* The callback run before the original function.
	* @param callback: A pointer to the custom task function (display) which will act like a callback
	* @param characterIndex: The unique index of your character
	* @param shouldOverride: Specify if your custom task should override the original code from Character Importer, if unsure, let it to false.
	*/
	API void register_task_pre_disp(char_importer_pre_load_cb callback, uint16_t characterIndex, bool shouldOverride);
	/*@brief
	* Register a custom task function for your character, useful if you want to add extra code like gameplay for your character
	* The callback run AFTER the original function.
	* @param callback: A pointer to the custom task function (exec) which will act like a callback
	* @param characterIndex: The unique index of your character
	* @param shouldOverride: Specify if your custom task should override the original code from Character Importer, if unsure, let it to false.
	*/
	API void register_task_post_exec(char_importer_post_load_cb callback, uint16_t characterIndex);
	/*@brief
	* Register a custom task function for your character, useful if you want to add extra code like specific effect or animation in the display function.
	* The callback run AFTER the original function.
	* @param callback: A pointer to the custom task function (display) which will act like a callback
	* @param characterIndex: The unique index of your character
	* @param shouldOverride: Specify if your custom task should override the original code from Character Importer, if unsure, let it to false.
	*/
	API void register_task_post_disp(char_importer_post_load_cb callback, uint16_t characterIndex);
	/**
	* @brief Get the list of all added characters.
	* Return a vector that contains all the characters.
	*/
	API std::vector<NewChar>* GetNewCharactersList();



#ifdef __cplusplus
}
#endif

#undef API

#endif