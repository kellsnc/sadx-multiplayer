#include "pch.h"
#include "SADXModLoader.h"
#include "FastFunctionHook.hpp"
#include "multiplayer.h"
#include "sadx_utils.h"

FastUsercallHookPtr<void(*)(Flicky* __this, taskwk* twp), noret, rEDI, rESI> Flicky_detectAim(0x4D8A10);

void Flicky_detectAim_r(Flicky* __this, taskwk* twp)
{
	if ((twp->wtimer & 0x1000) && (twp->wtimer & 0x40))
		return;

	taskwk* ptwp = playertwp[GetClosestPlayerNum(&twp->pos)];

	if (!ptwp)
		return;

	switch (twp->mode)
	{
	case 0:
		__this->aim.x = ptwp->pos.x;
		__this->aim.y = ptwp->pos.y;
		__this->aim.z = ptwp->pos.z;
		break;
	case 1:
		if (!(twp->wtimer & 0x400))
		{
			if (__this->movefunc == (void(__cdecl*)(Flicky*, taskwk*))Function_Goma)
			{
				__this->aim = twp->pos;
			}
			else
			{
				__this->aim.x = ptwp->pos.x;
				__this->aim.y = ptwp->pos.y;
				__this->aim.z = ptwp->pos.z;
			}
			__this->targetradius = 10.0f;
			twp->wtimer |= 0x400;
		}

		if (twp->wtimer & 0x800)
		{
			if (__this->transvelocity.x <= 0.0f)
			{
				__this->aim.x += njRandom() * 3.0f + 1.0f;
			}
			else
			{
				__this->aim.x += njRandom() * 3.0f - 4.0f;
			}

			if (__this->transvelocity.z <= 0.0)
			{
				__this->aim.z += njRandom() * 3.0f + 1.0f;
			}
			else
			{
				__this->aim.z += njRandom() * 3.0f - 4.0f;
			}

			twp->wtimer &= ~0x800u;
		}
		break;
	case 2:
	case 3:
	case 4:
		Angle ang;

		if (twp->mode == 2)
			ang = 0;
		else if (twp->mode == 3)
			ang = 0x4000;
		else
			ang = 0xC000;
		
		if (twp->wtimer & 0x800)
		{
			twp->wtimer &= ~0x800;
			twp->mode = 2 - (njRandom() * -3.0f);
		}

		NJS_VECTOR v;
		v.x = twp->pos.x - ptwp->pos.x;
		v.y = twp->pos.y - ptwp->pos.y;
		v.z = twp->pos.z - ptwp->pos.z;

		if (v.z * v.z + v.y * v.y + v.x * v.x <= 25.0f)
		{
			v.y = njSin(0) * 8.0f;
			Float v9 = njCos(0) * 8.0f;
			v.x = njCos(ang) * v9;
			v.z = -(njSin(ang) * v9);
		}
		else
		{
			NJS_MATRIX m;
			njUnitVector(&v);
			njUnitMatrix(m);
			if (ang)
			{
				njRotateY(m, ang);
			}
			njCalcVector(m, &v, &v);
			v.x *= 8.0f;
			v.y *= 8.0f;
			v.z *= 8.0f;
		}

		__this->aim.x = v.x + twp->pos.x;
		__this->aim.y = v.y + twp->pos.y;
		__this->aim.z = v.z + twp->pos.z;
		break;
	}
}

void Flicky_detectAim_m(Flicky* __this, taskwk* twp)
{
	if (multiplayer::IsActive())
	{
		Flicky_detectAim_r(__this, twp);
	}
	else
	{
		Flicky_detectAim.Original(__this, twp);
	}
}

void patch_flicky_init()
{
	Flicky_detectAim.Hook(Flicky_detectAim_m);
}

RegisterPatch patch_flicky(patch_flicky_init);