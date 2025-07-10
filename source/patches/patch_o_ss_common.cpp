#include "pch.h"

void patch_o_ss_common_init()
{
	//hostel door
	WriteCall((void*)0x630902, GetCharacterID_r);

	//train station back door
	WriteCall((void*)0x638882, GetCharacterID_r);
	//train station entrance
	WriteCall((void*)0x63AB72, GetCharacterID_r);
	//train available
	WriteCall((void*)0x539C62, GetCharacterID_r);

	//Speed Highway Shutter
	WriteCall((void*)0x63A589, GetCharacterID_r);
	WriteCall((void*)0x63A2A2, GetCharacterID_r);

	//Speed Highway Elevator
	WriteCall((void*)0x638CC2, GetCharacterID_r);

	//Speed Highway Knuckles Entrance
	WriteCall((void*)0x636BF2, GetCharacterID_r);

	//barricade
	WriteCall((void*)0x6389F2, GetCharacterID_r);

	//Casino door and button 
	WriteCall((void*)0x6383E2, GetCharacterID_r);
	WriteCall((void*)0x6380B2, GetCharacterID_r);
	WriteCall((void*)0x6382CE, GetCharacterID_r);

	//car blocking the sewers
	WriteCall((void*)0x639742, GetCharacterID_r);

	//ice stone
	WriteCall((void*)0x637D29, GetCharacterID_r);
}

RegisterPatch patch_o_ss_common(patch_o_ss_common_init);