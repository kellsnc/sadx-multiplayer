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

}

RegisterPatch patch_o_mr_common(patch_o_mr_common_init);