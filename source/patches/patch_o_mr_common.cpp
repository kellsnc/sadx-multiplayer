#include "pch.h"

void patch_o_mr_common_init()
{
	//cave entry
	WriteCall((void*)0x534572, GetCharacterID_r);
	//wv entrance (set windstone flag to the right character)
	WriteCall((void*)0x53C5C2, GetCharacterID_r);
	
	//island door
	WriteCall((void*)0x53F302, GetCharacterID_r);
	WriteCall((void*)0x53F3A1, GetCharacterID_r);
	WriteCall((void*)0x53F206, GetCharacterID_r);

	//Red Mountain door
	WriteCall((void*)0x53E675, GetCharacterID_r);
	WriteCall((void*)0x53E5E9, GetCharacterID_r);

	//kiki cage
	WriteCall((void*)0x53F97D, GetCharacterID_r);
	WriteCall((void*)0x53FE54, GetCharacterID_r);
	WriteCall((void*)0x53FE89, GetCharacterID_r);
	WriteCall((void*)0x53FEA9, GetCharacterID_r);

	//lw temple
	WriteCall((void*)0x532E62, GetCharacterID_r);
	WriteCall((void*)0x532E8F, GetCharacterID_r);
	WriteCall((void*)0x53B6C2, GetCharacterID_r); //knux entrance


	//Sand Hill Entry
	WriteCall((void*)0x53EB16, GetCharacterID_r);
	WriteCall((void*)0x53EB56, GetCharacterID_r);
	WriteCall((void*)0x53EAF2, GetCharacterID_r);

	//Angel Island door
	WriteCall((void*)0x53F922, GetCharacterID_r);

	//jungle cart
	WriteCall((void*)0x53DC77, GetCharacterID_r);

	//icestone
	WriteCall((void*)0x53C6B2, GetCharacterID_r);
	WriteCall((void*)0x532343, GetCharacterID_r); //idk some key related

	//final egg base entrance (change cmp == 1 to != 0)
	WriteData<1>((uint8_t*)0x53EF22, 0x0);
	WriteData<1>((uint8_t*)0x53EF24, 0x84);
	//final egg base entrance (close door if player is too far)
	WriteData<1>((uint8_t*)0x53EF93, 0x0);
	WriteData<1>((uint8_t*)0x53EF95, 0x85);

	//scene changer, hack change cmp == 1 to != 0
	WriteData<1>((uint8_t*)0x5395BE, 0x0);
	WriteData<1>((uint8_t*)0x5395BF, 0x74);

	//make everyone able to stay on final egg base (finalway)
	WriteData<1>((uint8_t*)0x5386AF, 0x0);
	WriteData<1>((uint8_t*)0x5386B0, 0x74);

}

RegisterPatch patch_o_mr_common(patch_o_mr_common_init);