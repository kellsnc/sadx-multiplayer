#pragma once

namespace fog
{
	bool GetUserFog(int pnum, ___stcFog* pFog);
	void SetUserFog(int pnum, ___stcFog* pFog);
	void ResetUserFog(int pnum);
}

void InitFogPatches();
