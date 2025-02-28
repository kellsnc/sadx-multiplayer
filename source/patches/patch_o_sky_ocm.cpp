#include "pch.h"
#include "SADXModLoader.h"

static OCMDATA ocmdataarray_m[SCHAR_MAX];

void patch_sky_ocm_init()
{
	WriteData((int8_t*)0x5FE33C, (int8_t)SCHAR_MAX);
	WriteData((int8_t*)0x5FE3A0, (int8_t)SCHAR_MAX);
	WriteData((int8_t*)0x5FE3EC, (int8_t)SCHAR_MAX);
	WriteData((int8_t*)0x5FE3EC, (int8_t)SCHAR_MAX);
	WriteData((int8_t*)0x5FE550, (int8_t)SCHAR_MAX);
	WriteData((int32_t*)0x5FE45B, SCHAR_MAX);
	WriteData((OCMDATA**)0x5FE2F9, ocmdataarray_m);
	WriteData((OCMDATA**)0x5FE381, ocmdataarray_m);
	WriteData((OCMDATA**)0x5FE3BC, ocmdataarray_m);
	WriteData((OCMDATA**)0x5FE454, ocmdataarray_m);
	WriteData((OCMDATA**)0x5FE4E3, ocmdataarray_m);
}

RegisterPatch patch_sky_ocm(patch_sky_ocm_init);