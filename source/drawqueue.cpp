#include "pch.h"
#include <math.h>
#include <vector>
#include <algorithm>
#include <d3d8types.h>
#include <d3d8.h>
#include <SADXModLoader.h>
#include <Trampoline.h>
#include "drawqueue.h"
#include "d3d8vars.h"
#include "splitscreen.h"
#include "camera.h"

/*

Rewrite the whole draw queue display just to support viewports.

*/

Trampoline* late_addZEntry_t = nullptr;
Trampoline* late_setOdr_t = nullptr;
Trampoline* late_exec_t = nullptr;

void njSetScreenDist_(Angle bams)
{
	if (ds_perspective_value != bams)
	{
		njSetScreenDist(bams);
	}
}

void DrawQueueItem_SetDrawParams(LATE_RQ_T* data)
{
	// Rendering flags
	_nj_control_3d_flag_ = data->rq.ctrl3dFlg;
	_nj_constant_attr_and_ = data->rq.atrAnd;
	_nj_constant_attr_or_ = data->rq.atrOr | NJD_FLAG_DOUBLE_SIDE;

	// Fog
	if ((data->rq.typ & QueuedModelFlags_FogEnabled))
	{
		if (gu8FogEnbale == FALSE)
		{
			___njFogEnable();
		}
	}
	else
	{
		if (gu8FogEnbale == TRUE)
		{
			___njFogDisable();
		}
	}

	// Palette
	int palette = data->rq.typ >> 6;
	if (palette != -1)
	{
		if (data->rq.no & 0x8000)
		{
			_nj_constant_attr_or_ |= NJD_FLAG_IGNORE_LIGHT | NJD_FLAG_IGNORE_SPECULAR;
			palette = 0;
		}

		if (palette != lig_curGjPaletteNo___)
		{
			___dsSetPalette(palette * 2);
		}
		
		lig_setLight4gjpalNo(palette);
	}

	// Texture
	CurrentTexList = data->rq.texLst;
	if (!VerifyTexList(data->rq.texLst))
	{
		Direct3D_SetTexList(data->rq.texLst);
	}

	// Depth
	Direct3D_SetZFunc(3u);
	if ((data->rq.typ & QueuedModelFlags_ZTestWrite) != 0)
	{
		njSetZUpdateMode(1u);
	}
	else
	{
		njSetZUpdateMode(0);
	}

	// Material
	njSetConstantMaterial(&data->rq.argb);
	njColorBlendingMode_(NJD_SOURCE_COLOR, (data->rq.alpMd & 0xF));
	njColorBlendingMode_(NJD_DESTINATION_COLOR, (data->rq.alpMd >> 4));
}

void DrawQueueItem_SetViewPort(LATE_RQ_T* data)
{
	int viewport = (BYTEn(data->rq.ctrl3dFlg, 3) & ~0x80) - 1;

	if (SplitScreen::ChangeViewPort(viewport))
	{
		ApplyMultiCamera(viewport < 0 ? 0 : viewport);
	}
}

void DrawQueue_DrawItem(LATE_RQ_T* data)
{
	int no = data->rq.no & 0x7FFF;

	if (no < 0)
	{
		return;
	}

	DrawQueueItem_SetViewPort(data); // <- custom
	DrawQueueItem_SetDrawParams(data);

	auto perspective = _nj_screen_.dist;

	switch (data->rq.typ & 0xF)
	{
	case QueuedModelType_BasicModel:
		njSetTexture(data->rq.texLst);
		njSetMatrix(0, data->obj.mtx);
		DrawModelThing((NJS_MODEL_SADX*)data->rq.etc);
		break;
	case QueuedModelType_Sprite2D:
		if (!isTextureNG(data->s2d.spr.tlist))
		{
			njSetScreenDist_(0x31C7);
			njSetMatrix(0, data->s2d.mtx);
			njDrawSprite2D_DrawNow(&data->s2d.spr, no, data->s2d.pri > -1.0f ? -1.0f : data->s2d.pri, data->s2d.atr);
		}
		break;
	case QueuedModelType_Sprite3D:
		if (!isTextureNG(data->s3d.spr.tlist))
		{
			njSetScreenDist_(0x31C7);
			njSetMatrix(0, data->s3d.mtx);
			njDrawSprite3D_DrawNow(&data->s3d.spr, no, data->s3d.atr);
		}
		break;
	case QueuedModelType_Line3D:
		if (data->po3d.atr & NJD_USE_TEXTURE && data->rq.texLst)
		{
			njSetTextureNum_(data->po3d.texNum);
		}
		njSetScreenDist_(0x31C7);
		njSetMatrix(0, data->po3d.mtx);
		njDrawLine3D(&data->po3d.p3c, no, data->po3d.atr);
		break;
	case QueuedModelType_3DLinesMaybe:
		if (data->po3d.atr & NJD_USE_TEXTURE && data->rq.texLst)
		{
			njSetTextureNum_(data->po3d.texNum);
		}
		njSetScreenDist_(0x31C7);
		njSetMatrix(0, data->po3d.mtx);
		njDrawTriangle3D(&data->po3d.p3c, no, data->po3d.atr);
		break;
	case QueuedModelType_2DLinesMaybe:
		if (data->po2d.atr & NJD_USE_TEXTURE && data->rq.texLst)
		{
			njSetTextureNum_(data->po2d.texNum);
		}
		njSetScreenDist_(0x31C7);
		njSetMatrix(0, data->po2d.mtx);
		___SAnjDrawPolygon2D(&data->po2d.p2c, no, data->po2d.pri > -1.0f ? -1.0f : data->po2d.pri, data->po2d.atr);
		break;
	case QueuedModelType_3DTriFanThing:
		if (data->po3d.atr & NJD_USE_TEXTURE && data->rq.texLst)
		{
			njSetTextureNum_(data->po3d.texNum);
		}
		njSetScreenDist_(0x31C7);
		njSetMatrix(0, data->po3d.mtx);
		njDrawPolygon3D(&data->po3d.p3c, no, data->po3d.atr);
		break;
	case QueuedModelType_ActionPtr:
		njSetMatrix(0, data->act.mtx);
		DrawAction(data->act.act, data->act.frm, LATE_MAT, 0.0f, DrawModelThing);
		break;
	case QueuedModelType_Rect:
		njSetScreenDist_(0x31C7);
		ds_DrawBoxFill2D(data->box2d.x, data->box2d.y, data->box2d.x2, data->box2d.y2, data->box2d.pri, data->box2d.argb);
		break;
	case QueuedModelType_Object:
		njSetTexture(data->rq.texLst);
		njSetMatrix(0, data->obj.mtx);
		dsDrawObject((NJS_OBJECT*)data->rq.etc);
		break;
	case QueuedModelType_Action:
		njSetMatrix(0, data->act.mtx);
		DrawActionB(data->act.act, data->act.frm, 0, 0.0f, DrawModelThing);
		break;
	case QueuedModelType_Callback:
		if (data->func.fnc)
		{
			njSetMatrix(0, data->func.mtx);
			njSetZCompare(1);
			njSetZUpdateMode(1);
			(data->func.fnc)(data->func.arg);
		}
		break;
	case QueuedModelType_TextureMemList:
		njSetScreenDist_(0x31C7);
		njSetMatrix(0, data->tex.mtx);
		njDrawTextureMemList(data->tex.vtx, data->tex.num, data->tex.ind, data->tex.tf);
		break;
	case QueuedModelType_Line2D:
		if (data->po2d.atr & NJD_USE_TEXTURE && data->rq.texLst)
		{
			njSetTextureNum_(data->po2d.texNum);
		}
		njSetScreenDist_(0x31C7);
		njSetMatrix(0, data->po2d.mtx);
		njDrawLine2D(&data->po2d.p2c, no, data->po2d.pri > -1.0f ? -1.0f : data->po2d.pri, data->po2d.atr);
		break;
	case QueuedModelType_MotionThing:
		njSetMatrix(0, data->shpmot.mtx);
		DrawShapeMotion(data->shpmot.obj, data->shpmot.mot, data->shpmot.shp, data->shpmot.frm, (LATE)data->shpmot.flgs, data->shpmot.clpScl, data->shpmot.drwMdl);
		break;
	}

	_nj_screen_.dist = perspective;
}

void __cdecl late_exec_r()
{
	if (SplitScreen::IsActive())
	{
		NJS_MATRIX orig_matrix;
		njGetMatrix(orig_matrix);
		Direct3D_SetZFunc(3u);

		if (loop_count || late_execMode)
		{
			if (late_execMode == 2)
			{
				njSetZCompare(7);
				ds_DrawBoxFill2D(0.0f, 0.0f, static_cast<float>(HorizontalResolution), static_cast<float>(VerticalResolution), -1.0f, 0xFF000000);
				npSetZCompare();
			}

			late_execMode = 0;
			return;
		}

		auto _ds_perspective_value = ds_perspective_value;
		auto _lig_curGjPaletteNo___ = lig_curGjPaletteNo___;
		auto __nj_control_3d_flag_ = _nj_control_3d_flag_;
		auto __nj_constant_attr_and_ = _nj_constant_attr_and_;
		auto __nj_constant_attr_or_ = _nj_constant_attr_or_;
		bool fog = gu8FogEnbale == TRUE;
		bool fogemu = (gFogEmu.u8Emulation & 3);

		ResetMaterial();

		if (lig_curGjPaletteNo___)
		{
			___dsSetPalette(0);
		}

		for (int i = 0; i < 2048; ++i)
		{
			auto inst = DrawQueuePtr[i].next;
			auto self_ptr = &DrawQueuePtr[i];

			while (inst != self_ptr)
			{
				DrawQueue_DrawItem((LATE_RQ_T*)inst);
				inst = inst->next;
			}
		}

		SplitScreen::ChangeViewPort(-1);
		njSetMatrix(0, orig_matrix);
		njds_texList = 0;
		___njSetConstantMaterial(&cur_argb);

		if (lig_curGjPaletteNo___ != _lig_curGjPaletteNo___)
		{
			___dsSetPalette(_lig_curGjPaletteNo___ * 2);
		}

		lig_resetGjPaletteNo___(_lig_curGjPaletteNo___);
		lig_setLight4gjpalNo(_lig_curGjPaletteNo___);
		njSetZCompare(1);
		njSetZUpdateMode(1);
		_nj_control_3d_flag_ = __nj_control_3d_flag_;
		_nj_constant_attr_and_ = __nj_constant_attr_and_;
		_nj_constant_attr_or_ = __nj_constant_attr_or_;
		njSetScreenDist_(_ds_perspective_value);

		if (fog)
		{
			___njFogEnable();
		}
		else
		{
			___njFogDisable();
		}
	}
	else
	{
		TARGET_DYNAMIC(late_exec)();
	}
}

LATE_RQ* __cdecl late_setOdr_o(__int16 no, int sz, QueuedModelType odr, QueuedModelFlagsB flgs)
{
	const auto late_setOdr_ptr = late_setOdr_t->Target();

	LATE_RQ* result;

	__asm
	{
		push dword ptr[flgs]
		push dword ptr[odr]
		push[sz]
		movzx ax, [no]
		call late_setOdr_ptr
		add esp, 12
		mov result, eax
	}

	return result;
}

static LATE_RQ* late_setOdr_r(__int16 no, int sz, QueuedModelType odr, QueuedModelFlagsB flgs)
{
	auto rq = late_setOdr_o(no, sz, odr, flgs);

	if (rq && SplitScreen::IsActive() && SplitScreen::numViewPort != -1)
	{
		BYTEn(rq->ctrl3dFlg, 3) += SplitScreen::numViewPort + 1; // place viewport id in the last available digits
	}

	return rq;
}

static void __declspec(naked) late_setOdr_w()
{
	__asm
	{
		push[esp + 0Ch] // flgs
		push[esp + 0Ch] // odr
		push[esp + 0Ch] // sz
		movzx eax, ax
		push eax
		call late_setOdr_r
		add esp, 16
		retn
	}
}

void DrawQueue_Init()
{
	late_exec_t = new Trampoline(0x4086F0, 0x4086F6, late_exec_r);
	late_setOdr_t = new Trampoline(0x403F60, 0x403F65, late_setOdr_w);
	
	// Expand draw queue memory pool
	WriteData((Uint32*)0x408643, 0x100000ui32);
	WriteData((Uint32*)0x40864D, 0x100000ui32);
	WriteData((Uint32*)0x40866B, 0x100000ui32);
}
