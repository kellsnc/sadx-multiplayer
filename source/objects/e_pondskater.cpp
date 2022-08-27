#include "pch.h"
#include "SADXModLoader.h"
#include "Trampoline.h"

static void __cdecl PondDisplayer_r(task* tp);
Trampoline PondDisplayer_t(0x7AA3D0, 0x7AA3D5, PondDisplayer_r);
static void __cdecl PondDisplayer_r(task* tp)
{
    if (multiplayer::IsActive())
    {
        if (tp->twp->mode < 4)
        {
            PondDraw(tp->twp, (enemywk*)tp->mwp);
        }
    }
    else
    {
        TARGET_STATIC(PondDisplayer)(tp);
    }
}

static FunctionHook<void, task*> PondExec_t(0x7AA870);

void PondExec_r(task* obj)
{
	taskwk* data = obj->twp;
	ObjectData2* objdata2 = (ObjectData2*)obj->mwp;

	if (data && objdata2) {

		if (data->mode < 3) {

			if (OhNoImDead((EntityData1*)data, objdata2))
			{
				data->mode = 3;
				data->counter.b[1] = 0;
				data->counter.b[2] = 1;
				data->wtimer = 0;
				data->scl.z = 0.34999999f;
				return;
			}

		}
	}

	PondExec_t.Original(obj);
}


void PatchPondSkater()
{
    WriteData<5>((void*)0x7AA627, 0x90ui8); // remove redundant SetVelocityP
    WriteData<5>((void*)0x7AA7A9, 0x90ui8); // remove redundant SetVelocityP
    WriteData<5>((void*)0x7AA707, 0x90ui8); // remove redundant SetVelocityP

	PondExec_t.Hook(PondExec_r); //patch a crash when speeps are killed too far from player 1
}
