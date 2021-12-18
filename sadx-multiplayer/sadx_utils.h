#pragma once

#define TASKWK_PLAYERID(a) a->counter.b[0]
#define TASKWK_CHARID(a) a->counter.b[1]

struct SONIC_INPUT
{
	int angle;
	float stroke;
};

VoidFunc(DisplayTask, 0x40B540);
TaskFunc(Camera, 0x438090);
DataPointer(taskwk*, camera_twp, 0x3B2CBB0);
FunctionPointer(void, ds_DrawBoxFill2D, (float x, float y, float x2, float y2, float pri, int argb), 0x4071C0);
DataPointer(int, loop_count, 0x3B1117C);
DataPointer(Uint8, ucInputStatus, 0x909FB0);
DataArray(Uint8, ucInputStatusForEachPlayer, 0x909FB4, 4);
DataArray(SONIC_INPUT, input_data, 0x3B0E368, 8);
DataArray(SONIC_INPUT, input_dataG, 0x3B0E7A0, 8);
VoidFunc(FreeQueueSound, 0x424460);
DataArray(PDS_PERIPHERAL*, per, 0x3B0E77C, 8);
DataArray(PDS_PERIPHERAL, perG, 0x3B0E7F0, 8);
VoidFunc(sub_464DF0, 0x464DF0);
ObjectFunc(sub_425B30, 0x425B30);
ObjectFunc(sub_425BB0, 0x425BB0);