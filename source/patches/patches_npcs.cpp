#include  "pch.h"

FastFunctionHook<void> LoadMRNPCsMainChara_h(0x52F140);
FastFunctionHook<void> LoadSSNPCsMainChara_h(0x62F5D0);
FastFunctionHook<void> LoadEC0NPCsMainChara_h(0x51AE00);
FastFunctionHook<void> LoadEC3NPCsMainChara_h(0x525600);

void __cdecl LoadMRNPCsMainChara_r()
{
	if (multiplayer::IsActive())
	{
		return;
	}

	LoadMRNPCsMainChara_h.Original();
}

void __cdecl LoadSSNPCsMainChara_r()
{
	if (multiplayer::IsActive())
	{
		return;
	}

	LoadSSNPCsMainChara_h.Original();
}

void __cdecl LoadEC0NPCsMainChara_r()
{
	if (multiplayer::IsActive())
	{
		return;
	}

	LoadEC0NPCsMainChara_h.Original();
}

void __cdecl LoadEC3NPCsMainChara_r()
{
	if (multiplayer::IsActive())
	{
		return;
	}

	LoadEC3NPCsMainChara_h.Original();
}

void patch_npcs_init()
{
	LoadMRNPCsMainChara_h.Hook(LoadMRNPCsMainChara_r);
	LoadSSNPCsMainChara_h.Hook(LoadSSNPCsMainChara_r);
	LoadEC0NPCsMainChara_h.Hook(LoadEC0NPCsMainChara_r);
	LoadEC3NPCsMainChara_h.Hook(LoadEC3NPCsMainChara_r);
}

RegisterPatch patch_npcs(patch_npcs_init);