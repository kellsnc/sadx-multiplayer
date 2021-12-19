#include "pch.h"
#include <vector>
#include <array>
#include <d3d8types.h>
#include <d3d8.h>
#include "drawqueue.h"
#include "splitscreen.h"

// Based on SF94's sprite-pass
// https://github.com/michael-fadely/sadx-sprite-pass

DataPointer(IDirect3DDevice8*, Direct3D_Device, 0x03D128B0);
DataPointer(D3DVIEWPORT8, Direct3D_ViewPort, 0x03D12780);

Trampoline* njDrawSprite2D_t = nullptr;

struct QueuedSprite
{
	NJS_SPRITE sp;
	NJS_TEXANIM tanim{};
	Float pri;
	NJD_SPRITE attr;
	DWORD srcblend, dstblend;
	NJS_MATRIX transform{};
	NJS_ARGB nj_constant_material_{};
	bool fog = false;
	int numviewport;

	QueuedSprite(const NJS_SPRITE* sp, Int n, Float pri, NJD_SPRITE attr) : sp(*sp), pri(pri), attr(attr)
	{
		nj_constant_material_ = _nj_constant_material_;
		numviewport = numViewPort;
		fog = FogEnabled;

		Direct3D_Device->GetRenderState(D3DRENDERSTATETYPE::D3DRS_SRCBLEND, &srcblend);
		Direct3D_Device->GetRenderState(D3DRENDERSTATETYPE::D3DRS_DESTBLEND, &dstblend);

		if (sp->tlist == nullptr)
		{
			this->sp.tlist = CurrentTexList;
		}

		this->tanim = sp->tanim[n];

		njGetMatrix(transform);
	}

	void apply()
	{
		_nj_constant_material_ = nj_constant_material_;

		Direct3D_Device->SetRenderState(D3DRENDERSTATETYPE::D3DRS_SRCBLEND, srcblend);
		Direct3D_Device->SetRenderState(D3DRENDERSTATETYPE::D3DRS_DESTBLEND, dstblend);

		if (fog)
		{
			ToggleStageFog();
		}
		else
		{
			DisableFog();
		}

		ChangeViewPort(numviewport);
		njSetMatrix(nullptr, transform);

		this->sp.tanim = &this->tanim;
	}
};

static std::vector<QueuedSprite> sprites_2d;

void __cdecl njDrawSprite2D_r(NJS_SPRITE* sp, Int n, Float pri, NJD_SPRITE attr)
{
	if (sp && sp->tlist)
	{
		sprites_2d.emplace_back(sp, n, pri, attr);
	}
}

template<class T>
bool compare(T i1, T i2)
{
	return (i1.pri > i2.pri);
}

void draw_sprites_2d()
{
	Direct3D_EnableZWrite(FALSE);
	Direct3D_SetZFunc(7);
	SetDefaultAlphaBlend();

	sort(sprites_2d.begin(), sprites_2d.end(), compare<QueuedSprite>);

	njPushMatrixEx();
	for (auto& sprite : sprites_2d)
	{
		sprite.apply();
		TARGET_DYNAMIC(njDrawSprite2D)(&sprite.sp, 0, sprite.pri, sprite.attr);
	}
	njPopMatrixEx();

	if (FogEnabled)
	{
		ToggleStageFog();
	}
	else
	{
		DisableFog();
	}

	sprites_2d.clear();

	ResetViewPort();
	Direct3D_EnableZWrite(FALSE);
	Direct3D_SetZFunc(7);
	SetDefaultAlphaBlend();
	DrawDebugText(1);
	SetDefaultAlphaBlend();
}

void InitDrawQueue()
{
	njDrawSprite2D_t = new Trampoline(0x0077E050, 0x0077E058, njDrawSprite2D_r);

	sprites_2d.reserve(512);
	WriteData<5>((void*)0x0078B9E3, 0x90i8);
}

void RunDrawQueue()
{
	draw_sprites_2d();
}