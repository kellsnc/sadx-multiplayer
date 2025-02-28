#include "pch.h"
#include "SADXModLoader.h"
#include "RegisterPatch.hpp"
#include "multiplayer.h"

// In rare occasions the game checks if objects are visible in the exec function
// However dsCheckViewV uses the current world matrix, so it only works for player 1
// Since this is rare and hardly improving performance, we just always pass in multiplayer

BOOL __cdecl dsCheckViewV_ExecHack(NJS_POINT3* ft, float radius)
{
	if (multiplayer::IsActive())
	{
		return true;
	}
	else
	{
		return dsCheckViewV(ft, radius);
	}
}

void patch_draw_init()
{
	WriteCall((void*)0x4E138F, dsCheckViewV_ExecHack); // wv hane, bigfloot, saku...
	WriteCall((void*)0x5D3D54, dsCheckViewV_ExecHack); // ObjectCasinoCris
	WriteCall((void*)0x5E8327, dsCheckViewV_ExecHack); // ObjectRuinFire
	WriteCall((void*)0x4AB41A, dsCheckViewV_ExecHack); // EnemySaru
	WriteCall((void*)0x7AA307, dsCheckViewV_ExecHack); // EnemyPondSkater
}

RegisterPatch patch_draw(patch_draw_init);