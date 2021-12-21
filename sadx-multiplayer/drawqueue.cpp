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

Rewrite of the draw queue system to support viewports
Uses vector instead of linked list

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

struct CUSTOM_LATE_RQ
{
    LATE_RQ_T data{};
    int viewport;

    int GetDrawOrder()
    {
        switch (data.rq.typ)
        {
        default:
        case QueuedModelType_BasicModel:
        case QueuedModelType_Action:
        case QueuedModelType_Object:
        case QueuedModelType_ActionPtr:
        case QueuedModelType_MotionThing:
            return 0;
        case QueuedModelType_3DLinesMaybe:
        case QueuedModelType_3DTriFanThing:
        case QueuedModelType_Sprite3D:
        case QueuedModelType_Line3D:
            return 1;
        case QueuedModelType_2DLinesMaybe:
        case QueuedModelType_Line2D:
        case QueuedModelType_Sprite2D:
        case QueuedModelType_TextureMemList:
        case QueuedModelType_Rect:
        case QueuedModelType_Callback:
            return 2;
        }
    }

    void SetDrawParams()
    {
        Direct3D_SetZFunc(3u);

        if ((data.rq.typ & QueuedModelFlags_ZTestWrite) != 0)
        {
            njSetZUpdateMode(1u);
        }
        else
        {
            njSetZUpdateMode(0);
        }

        njSetConstantMaterial(&data.rq.argb);
        njColorBlendingMode_(NJD_SOURCE_COLOR, (data.rq.alpMd & 0xF));
        njColorBlendingMode_(NJD_DESTINATION_COLOR, (data.rq.alpMd >> 4));

        CurrentTexList = data.rq.texLst;

        if (!VerifyTexList(data.rq.texLst))
        {
            njSetTexture(data.rq.texLst);
        }

        if ((data.rq.no & 0x7FFF) > 0)
        {
            if ((data.rq.typ & QueuedModelFlags_FogEnabled))
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
        }

        auto light_type = data.rq.typ >> 6;

        if (light_type != lig_curGjPaletteNo___)
        {
            ___dsSetPalette(light_type);
        }
    }

    void Draw()
    {
        if (ChangeViewPort(viewport))
        {
            ApplyMultiCamera(camera_twp, viewport);
        }

        SetDrawParams();

        int no = data.rq.no & 0x7FFF;

        switch (data.rq.typ & 0xF)
        {
        case QueuedModelType_BasicModel:
            njSetTexture(data.rq.texLst);
            njSetMatrix(0, data.obj.mtx);
            DrawModelThing((NJS_MODEL_SADX*)data.rq.etc);
            break;
        case QueuedModelType_Sprite2D:
            if (!isTextureNG(data.s2d.spr.tlist))
            {
                njSetScreenDist_(0x31C7);
                njSetMatrix(0, data.s2d.mtx);
                njDrawSprite2D_DrawNow(&data.s2d.spr, no, data.s2d.pri > -1.0f ? -1.0f : data.s2d.pri, data.s2d.atr);
            }
            break;
        case QueuedModelType_Sprite3D:
            if (!isTextureNG(data.s3d.spr.tlist))
            {
                njSetScreenDist_(0x31C7);
                njSetMatrix(0, data.s3d.mtx);
                njDrawSprite3D_DrawNow(&data.s3d.spr, no, data.s3d.atr);
            }
            break;
        case QueuedModelType_Line3D:
            if (data.po3d.atr & NJD_USE_TEXTURE && data.rq.texLst)
            {
                njSetTextureNum_(data.po3d.texNum);
            }
            njSetScreenDist_(0x31C7);
            njSetMatrix(0, data.po3d.mtx);
            njDrawLine3D(&data.po3d.p3c, no, data.po3d.atr);
            break;
        case QueuedModelType_3DLinesMaybe:
        case QueuedModelType_3DTriFanThing:
            if (data.po3d.atr & NJD_USE_TEXTURE && data.rq.texLst)
            {
                njSetTextureNum_(data.po3d.texNum);
            }
            njSetScreenDist_(0x31C7);
            njSetMatrix(0, data.po3d.mtx);
            njDrawPolygon3D(&data.po3d.p3c, no, data.po3d.atr);
            break;
        case QueuedModelType_2DLinesMaybe:
            if (data.po2d.atr & NJD_USE_TEXTURE && data.rq.texLst)
            {
                njSetTextureNum_(data.po2d.texNum);
            }
            njSetScreenDist_(0x31C7);
            njSetMatrix(0, data.po2d.mtx);
            ___SAnjDrawPolygon2D(&data.po2d.p2c, no, data.po2d.pri > -1.0f ? -1.0f : data.po2d.pri, data.po2d.atr);
            break;
        case QueuedModelType_ActionPtr:
            njSetMatrix(0, data.act.mtx);
            DrawAction(data.act.act, data.act.frm, 0, 0.0f, DrawModelThing);
            break;
        case QueuedModelType_Rect:
            njSetScreenDist_(0x31C7);
            ds_DrawBoxFill2D(data.box2d.x, data.box2d.y, data.box2d.x2, data.box2d.y2, data.box2d.pri, data.box2d.argb);
            break;
        case QueuedModelType_Object:
            njSetTexture(data.rq.texLst);
            njSetMatrix(0, data.obj.mtx);
            ___dsDrawObject((NJS_OBJECT*)data.rq.etc);
            break;
        case QueuedModelType_Action:
            njSetMatrix(0, data.act.mtx);
            DrawActionB(data.act.act, data.act.frm, 0, 0.0f, DrawModelThing);
            break;
        case QueuedModelType_Callback:
            if (data.func.fnc)
            {
                njSetMatrix(0, data.func.mtx);
                njSetZCompare(1);
                njSetZUpdateMode(1);
                (data.func.fnc)(data.func.arg);
            }
            break;
        case QueuedModelType_TextureMemList:
            njSetScreenDist_(0x31C7);
            njSetMatrix(0, data.tex.mtx);
            njDrawTextureMemList(data.tex.vtx, data.tex.num, data.tex.ind, data.tex.tf);
            break;
        case QueuedModelType_Line2D:
            if (data.po2d.atr & NJD_USE_TEXTURE && data.rq.texLst)
            {
                njSetTextureNum_(data.po2d.texNum);
            }
            njSetScreenDist_(0x31C7);
            njSetMatrix(0, data.po2d.mtx);
            njDrawLine2D(&data.po2d.p2c, no, data.po2d.pri > -1.0f ? -1.0f : data.po2d.pri, data.po2d.atr);
            break;
        case QueuedModelType_MotionThing:
            njSetMatrix(0, data.shpmot.mtx);
            DrawShapeMotion(data.shpmot.obj, data.shpmot.mot, data.shpmot.shp, data.shpmot.frm, data.shpmot.flgs, data.shpmot.clpScl, data.shpmot.drwMdl);
            break;
        }
    }

    CUSTOM_LATE_RQ(int no, size_t sz, int odr, QueuedModelFlagsB flgs)
    {
        this->data.rq.typ = odr | (32 * FogEnabled) | (16 * (flgs & QueuedModelFlagsB_EnableZWrite));
        this->data.rq.texLst = CurrentTexList;
        this->data.rq.no = no;
        this->data.rq.alpMd = AlphaBlendRelatedThing;
        this->data.rq.ctrl3dFlg = _nj_control_3d_flag_;
        this->data.rq.atrAnd = _nj_constant_attr_and_;
        this->data.rq.atrOr = _nj_constant_attr_or_;

        NJS_ARGB* argb_color = &DefaultSpriteColor;

        if ((flgs & QueuedModelFlagsB_NoColor) == 0)
        {
            argb_color = &GlobalSpriteColor;
        }

        this->data.rq.argb.a = argb_color->a;
        this->data.rq.argb.r = argb_color->r;
        this->data.rq.argb.g = argb_color->g;
        this->data.rq.argb.b = argb_color->b;

        if ((flgs & QueuedModelFlagsB_SomeTextureThing) != 0)
        {
            this->data.rq.no |= 0x8000;
        }

        this->data.rq.typ |= CurrentLightType << 6;

        this->viewport = numViewPort;
    }
};

std::vector<CUSTOM_LATE_RQ> custom_late_zentry;

bool compare(CUSTOM_LATE_RQ i1, CUSTOM_LATE_RQ i2)
{
    return (i1.data.rq.zkey > i2.data.rq.zkey && i1.GetDrawOrder() > i2.GetDrawOrder());
}

void __cdecl late_exec_r()
{
    if (IsMultiplayerEnabled())
    {
        NJS_MATRIX orig_matrix;
        njGetMatrix(orig_matrix);
        Direct3D_SetZFunc(3u);

        if (loop_count || late_execMode)
        {
            if (late_execMode == 2)
            {
                njSetZCompare(7);
                ds_DrawBoxFill2D(0.0f, 0.0f, HorizontalResolution, VerticalResolution, -1.0f, 0xFF000000);
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

        sort(custom_late_zentry.begin(), custom_late_zentry.end(), compare);

        for (auto& item : custom_late_zentry)
        {
            item.Draw();

            if (_ds_perspective_value != ds_perspective_value)
            {
                njSetScreenDist(_ds_perspective_value);
            }
        }

        custom_late_zentry.clear();

        ChangeViewPort(-1);
        njSetMatrix(0, orig_matrix);
        njds_texList = 0;
        ___njSetConstantMaterial(&cur_argb);
        ___dsSetPalette(0);

        if (lig_curGjPaletteNo___ != _lig_curGjPaletteNo___)
        {
            ___dsSetPalette(_lig_curGjPaletteNo___);
            lig_resetGjPaletteNo___(_lig_curGjPaletteNo___);
        }

        ScaleVectorThing_Restore();
        njSetZCompare(1);
        njSetZUpdateMode(1);
        _nj_control_3d_flag_ = __nj_control_3d_flag_;
        _nj_constant_attr_and_ = __nj_constant_attr_and_;
        _nj_constant_attr_or_ = __nj_constant_attr_or_;
        njSetScreenDist(_ds_perspective_value);

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

void __cdecl late_addZEntry_o(LATE_RQ* rq, float z, LATE_LNK a)
{
    const auto late_addZEntry_ptr = late_addZEntry_t->Target();

    __asm
    {
        push dword ptr[a]
        push dword ptr[z]
        mov esi, [rq]
        call late_addZEntry_ptr
        add esp, 8
    }
}

void __cdecl late_addZEntry_r(LATE_RQ* rq, float z, LATE_LNK a)
{
    if (IsMultiplayerEnabled())
    {
        if (rq)
        {
            rq->zkey = z + DrawQueueDepthBias + 2048.0f;
        }
    }
    else
    {
        TARGET_DYNAMIC(late_exec)();
    }
}

static void __declspec(naked) late_addZEntry_w()
{
    __asm
    {
        push[esp + 08h] // a
        push[esp + 08h] // z
        push esi // rq
        call late_addZEntry_r
        pop esi
        add esp, 8
        retn
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
    if (IsMultiplayerEnabled())
    {
        custom_late_zentry.emplace_back(no, sz, odr, flgs);
        return &custom_late_zentry.at(custom_late_zentry.size() - 1).data.rq;
    }
    else
    {
        return late_setOdr_o(no, sz, odr, flgs);
    }
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
    WriteData((char*)0x407330, (char)0xC3);
    late_exec_t = new Trampoline(0x4086F0, 0x4086F6, late_exec_r);
    late_setOdr_t = new Trampoline(0x403F60, 0x403F65, late_setOdr_w);
    late_addZEntry_t = new Trampoline(0x403E80, 0x403E87, late_addZEntry_w);

    custom_late_zentry.reserve(1024);
}