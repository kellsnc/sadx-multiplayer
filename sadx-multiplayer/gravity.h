#pragma once

namespace gravity
{
	void SaveGlobalGravity();
	void SwapGlobalToUserGravity(int pnum);
	void RestoreGlobalGravity();

	bool GetUserGravity(int pnum, NJS_POINT3* v, Angle* angx, Angle* angz);
	void SetUserGravity(Angle angx, Angle angz, int pnum);
	void ResetUserGravity(int pnum);
}

void InitGravityPatches();