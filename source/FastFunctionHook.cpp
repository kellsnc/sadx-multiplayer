#include "pch.h"
#include "hde32.h"
#include "FastFunctionHook.hpp"

static uint8_t* currentpage = nullptr;
static int pageoffset = 0;
static int pagesize = -1;
static uint8_t* AllocateFunction(int sz)
{
	if (pagesize == -1)
	{
		SYSTEM_INFO sysinf;
		GetNativeSystemInfo(&sysinf);
		pagesize = sysinf.dwPageSize;
	}
	if (currentpage == nullptr || (pageoffset + sz) > pagesize)
	{
		currentpage = (uint8_t*)VirtualAlloc(nullptr, pagesize, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		pageoffset = 0;
		assert(currentpage != nullptr);
	}
	uint8_t* result = &currentpage[pageoffset];
	pageoffset += sz;
	if (pageoffset % 0x10 != 0)
		pageoffset += 0x10 - (pageoffset % 0x10);
	return result;
}

void* GenerateTrampoline(uint8_t* origbytes)
{
	// Todo: better way of managing relative addresses + also manage conditional jumps
	int callloc = -1;
	int jumploc = -1;

	int totalsize = 0;
	while (totalsize < 5)
	{
		hde32s hs;
		hde32_disasm(&origbytes[totalsize], &hs);

		if (hs.flags & F_ERROR)
		{
			throw std::exception("Fatal error while hooking function.");
		}

		switch (hs.opcode)
		{
		case 0xC2:
		case 0xC3: //retn
			throw std::exception("Function is too small for hooking");
		case 0xE8: // call
			callloc = totalsize;
			break;
		case 0xE9: // jump
		case 0xEB: // jump
			jumploc = totalsize;
			break;
		}

		totalsize += hs.len;
	}

	if (totalsize > 20)
	{
		throw std::exception("Trampoline is too large.");
	}

	// Create function to call original without hook
	auto func = AllocateFunction(totalsize + 5);
	memcpy(func, origbytes, totalsize); // Write original instructions to trampoline
	WriteJump(&func[totalsize], &origbytes[totalsize]); // Append jump from trampoline function to original

	// Fix any call/jump in the trampoline
	if (callloc != -1)
	{
		intptr_t addr = (intptr_t)&origbytes[callloc] + 5 + *reinterpret_cast<intptr_t*>(&origbytes[callloc + 1]);
		WriteCall(&func[callloc], (void*)addr);
	}
	else if (jumploc != -1)
	{
		intptr_t addr = (intptr_t)&origbytes[jumploc] + 5 + *reinterpret_cast<intptr_t*>(&origbytes[jumploc + 1]);
		WriteJump(&func[jumploc], (void*)addr);
	}

	return func;
}

void* GenerateThiscallWrapper(void* address, int stackcnt)
{
	int memsz = 10; // size of mov ecx, call and retn

	for (int i = 0; i < stackcnt; ++i)
	{
		memsz += 4;
	}

	auto codeData = AllocateFunction(memsz);
	int cdoff = 0;
	uint8_t stackoff = (stackcnt + 1) * 4; // stack+this

	for (int i = 0; i < stackcnt; ++i)
	{
		writebytes(codeData, cdoff, 0xFF, 0x74, 0x24, stackoff); // stack
	}

	writebytes(codeData, cdoff, 0x8B, 0x4C, 0x24, stackoff); // ECX

	WriteCall(&codeData[cdoff], address);
	cdoff += 5;

	codeData[cdoff++] = 0xC3;
	assert(cdoff == memsz);
	return codeData;
}

void GenerateThiscallHook(void* func, void* address, int stackcnt)
{
	int memsz = stackcnt > 0 ? 13 : 8; // size of push ecx, pop ecx, call and retn purge (and pop stack)

	for (int i = 0; i < stackcnt; ++i)
	{
		memsz += 4;
	}

	auto codeData = AllocateFunction(memsz);
	int cdoff = 0;
	uint8_t stackoff = stackcnt * 4;

	for (int i = 0; i < stackcnt; ++i)
	{
		writebytes(codeData, cdoff, 0xFF, 0x74, 0x24, stackoff); // stack
	}

	codeData[cdoff++] = 0x51; // PUSH ECX

	WriteCall(&codeData[cdoff], func);
	cdoff += 5;

	codeData[cdoff++] = 0x59; // POP ECX

	if (stackcnt > 0)
	{
		writebytes(codeData, cdoff, 0x83, 0xC4, (uint8_t)(stackcnt * 4)); // pop stack
		writebytes(codeData, cdoff, 0xC2, stackoff, 0x00); // retn + callee cleanup
	}
	else
	{
		codeData[cdoff++] = 0xC3; // normal retn
	}

	assert(cdoff == memsz);
	if (*(uint8_t*)address == 0xE8)
		WriteCall((void*)address, codeData);
	else
		WriteJump((void*)address, codeData);
}

void* GenerateUsercallWrapperC(int ret, intptr_t address, int nbreg, ...)
{
	const size_t argc = nbreg;
	int8_t* argarray = (int8_t*)malloc(argc);
	if (!argarray)
		return nullptr;

	va_list(ap);
	va_start(ap, nbreg);
	for (int i = 0; i < nbreg; ++i)
		argarray[i] = va_arg(ap, int);
	va_end(ap);

	int stackcnt = 0;
	int memsz = 0;
	for (size_t i = 0; i < argc; ++i)
	{
		switch (argarray[i])
		{
		case rEAX:
		case rECX:
		case rEDX:
			memsz += 4;
			break;
		case rEBX:
		case rESI:
		case rEDI:
		case rEBP:
			memsz += 6;
			break;
		case rAX:
		case rCX:
		case rDX:
			memsz += 5;
			break;
		case rBX:
		case rSI:
		case rDI:
		case rBP:
			memsz += 7;
			break;
		case rAL:
		case rCL:
		case rDL:
		case rAH:
		case rCH:
		case rDH:
			memsz += 4;
			break;
		case rBL:
		case rBH:
			memsz += 6;
			break;
		case stack1:
		case stack2:
			memsz += 6;
			++stackcnt;
			break;
		case stack4:
			memsz += 4;
			++stackcnt;
			break;
		}
	}
	memsz += 5; // call
	if (stackcnt > 0)
		memsz += 3;
	switch (ret)
	{
	case rEBX:
	case rECX:
	case rEDX:
	case rESI:
	case rEDI:
	case rEBP:
		memsz += 2;
		break;
	case rAX:
	case rBX:
	case rCX:
	case rDX:
	case rSI:
	case rDI:
	case rBP:
	case rAL:
	case rBL:
	case rCL:
	case rDL:
	case rAH:
	case rBH:
	case rCH:
	case rDH:
		memsz += 3;
		break;
	case rst0:
		memsz += 8;
		break;
	}
	++memsz; // retn
	auto codeData = AllocateFunction(memsz);
	int cdoff = 0;
	uint8_t stackoff = argc * 4;
	for (size_t i = 0; i < argc; ++i)
	{
		switch (argarray[i])
		{
		case rEBX:
		case rBX:
		case rBH:
		case rBL:
			codeData[cdoff++] = 0x53;
			stackoff += 4;
			break;
		case rESI:
		case rSI:
			codeData[cdoff++] = 0x56;
			stackoff += 4;
			break;
		case rEDI:
		case rDI:
			codeData[cdoff++] = 0x57;
			stackoff += 4;
			break;
		case rEBP:
		case rBP:
			codeData[cdoff++] = 0x55;
			stackoff += 4;
			break;
		}
	}
	for (int i = argc - 1; i >= 0; --i)
	{
		switch (argarray[i])
		{
		case rEAX:
			writebytes(codeData, cdoff, 0x8B, 0x44, 0x24, stackoff);
			stackoff -= 4;
			break;
		case rEBX:
			writebytes(codeData, cdoff, 0x8B, 0x5C, 0x24, stackoff);
			stackoff -= 4;
			break;
		case rECX:
			writebytes(codeData, cdoff, 0x8B, 0x4C, 0x24, stackoff);
			stackoff -= 4;
			break;
		case rEDX:
			writebytes(codeData, cdoff, 0x8B, 0x54, 0x24, stackoff);
			stackoff -= 4;
			break;
		case rESI:
			writebytes(codeData, cdoff, 0x8B, 0x74, 0x24, stackoff);
			stackoff -= 4;
			break;
		case rEDI:
			writebytes(codeData, cdoff, 0x8B, 0x7C, 0x24, stackoff);
			stackoff -= 4;
			break;
		case rEBP:
			writebytes(codeData, cdoff, 0x8B, 0x6C, 0x24, stackoff);
			stackoff -= 4;
			break;
		case rAX:
			writebytes(codeData, cdoff, 0x66, 0x8B, 0x44, 0x24, stackoff);
			stackoff -= 4;
			break;
		case rBX:
			writebytes(codeData, cdoff, 0x66, 0x8B, 0x5C, 0x24, stackoff);
			stackoff -= 4;
			break;
		case rCX:
			writebytes(codeData, cdoff, 0x66, 0x8B, 0x4C, 0x24, stackoff);
			stackoff -= 4;
			break;
		case rDX:
			writebytes(codeData, cdoff, 0x66, 0x8B, 0x54, 0x24, stackoff);
			stackoff -= 4;
			break;
		case rSI:
			writebytes(codeData, cdoff, 0x66, 0x8B, 0x74, 0x24, stackoff);
			stackoff -= 4;
			break;
		case rDI:
			writebytes(codeData, cdoff, 0x66, 0x8B, 0x7C, 0x24, stackoff);
			stackoff -= 4;
			break;
		case rBP:
			writebytes(codeData, cdoff, 0x66, 0x8B, 0x6C, 0x24, stackoff);
			stackoff -= 4;
			break;
		case rAL:
			writebytes(codeData, cdoff, 0x8A, 0x44, 0x24, stackoff);
			stackoff -= 4;
			break;
		case rBL:
			writebytes(codeData, cdoff, 0x8A, 0x5C, 0x24, stackoff);
			stackoff -= 4;
			break;
		case rCL:
			writebytes(codeData, cdoff, 0x8A, 0x4C, 0x24, stackoff);
			stackoff -= 4;
			break;
		case rDL:
			writebytes(codeData, cdoff, 0x8A, 0x54, 0x24, stackoff);
			stackoff -= 4;
			break;
		case rAH:
			writebytes(codeData, cdoff, 0x8A, 0x64, 0x24, stackoff);
			stackoff -= 4;
			break;
		case rBH:
			writebytes(codeData, cdoff, 0x8A, 0x7C, 0x24, stackoff);
			stackoff -= 4;
			break;
		case rCH:
			writebytes(codeData, cdoff, 0x8A, 0x6C, 0x24, stackoff);
			stackoff -= 4;
			break;
		case rDH:
			writebytes(codeData, cdoff, 0x8A, 0x74, 0x24, stackoff);
			stackoff -= 4;
			break;
		case stack1:
			writebytes(codeData, cdoff, 0x0F, 0xBE, 0x44, 0x24, stackoff, 0x50);
			break;
		case stack2:
			writebytes(codeData, cdoff, 0x0F, 0xBF, 0x44, 0x24, stackoff, 0x50);
			break;
		case stack4:
			writebytes(codeData, cdoff, 0xFF, 0x74, 0x24, stackoff);
			break;
		}
	}
	WriteCall(&codeData[cdoff], (void*)address);
	cdoff += 5;
	if (stackcnt > 0)
		writebytes(codeData, cdoff, 0x83, 0xC4, (uint8_t)(stackcnt * 4));
	for (int i = argc - 1; i >= 0; --i)
	{
		switch (argarray[i])
		{
		case rEBX:
		case rBX:
		case rBL:
		case rBH:
			codeData[cdoff++] = 0x5B;
			break;
		case rESI:
		case rSI:
			codeData[cdoff++] = 0x5E;
			break;
		case rEDI:
		case rDI:
			codeData[cdoff++] = 0x5F;
			break;
		case rEBP:
		case rBP:
			codeData[cdoff++] = 0x5D;
			break;
		}
	}
	switch (ret)
	{
	case rEBX:
		writebytes(codeData, cdoff, 0x89, 0xD8);
		break;
	case rECX:
		writebytes(codeData, cdoff, 0x89, 0xC8);
		break;
	case rEDX:
		writebytes(codeData, cdoff, 0x89, 0xD0);
		break;
	case rESI:
		writebytes(codeData, cdoff, 0x89, 0xF0);
		break;
	case rEDI:
		writebytes(codeData, cdoff, 0x89, 0xF8);
		break;
	case rEBP:
		writebytes(codeData, cdoff, 0x89, 0xE8);
		break;
	case rAX:
		writebytes(codeData, cdoff, 0x0F, 0xBF, 0xC0);
		break;
	case rBX:
		writebytes(codeData, cdoff, 0x0F, 0xBF, 0xC3);
		break;
	case rCX:
		writebytes(codeData, cdoff, 0x0F, 0xBF, 0xC1);
		break;
	case rDX:
		writebytes(codeData, cdoff, 0x0F, 0xBF, 0xC2);
		break;
	case rSI:
		writebytes(codeData, cdoff, 0x0F, 0xBF, 0xC6);
		break;
	case rDI:
		writebytes(codeData, cdoff, 0x0F, 0xBF, 0xC7);
		break;
	case rBP:
		writebytes(codeData, cdoff, 0x0F, 0xBF, 0xC5);
		break;
	case rAL:
		writebytes(codeData, cdoff, 0x0F, 0xBE, 0xC0);
		break;
	case rBL:
		writebytes(codeData, cdoff, 0x0F, 0xBE, 0xC3);
		break;
	case rCL:
		writebytes(codeData, cdoff, 0x0F, 0xBE, 0xC1);
		break;
	case rDL:
		writebytes(codeData, cdoff, 0x0F, 0xBE, 0xC2);
		break;
	case rAH:
		writebytes(codeData, cdoff, 0x0F, 0xBE, 0xC4);
		break;
	case rBH:
		writebytes(codeData, cdoff, 0x0F, 0xBE, 0xC7);
		break;
	case rCH:
		writebytes(codeData, cdoff, 0x0F, 0xBE, 0xC5);
		break;
	case rDH:
		writebytes(codeData, cdoff, 0x0F, 0xBE, 0xC6);
		break;
	case rst0:
		writebytes(codeData, cdoff, 0xD9, 0x5C, 0x24, 0xFC, 0x8B, 0x44, 0x24, 0xFC);
		break;
	}
	codeData[cdoff++] = 0xC3;
#if USERCALLDEBUG
	char fn[MAX_PATH];
	sprintf_s(fn, "usercallwrapper@%08X.bin", address);
	auto fh = fopen(fn, "wb");
	fwrite(codeData, memsz, 1, fh);
	fclose(fh);
#endif
	assert(cdoff == memsz);

	free(argarray);
	return (void*)codeData;
}

void GenerateUsercallHookC(void* func, int ret, intptr_t address, int nbreg, ...)
{
	const size_t argc = nbreg;
	int8_t* argarray = (int8_t*)malloc(argc);
	if (!argarray)
		return;

	va_list(ap);
	va_start(ap, nbreg);
	for (int i = 0; i < nbreg; ++i)
		argarray[i] = va_arg(ap, int);
	va_end(ap);

	int stackcnt = 0;
	int memsz = 0;
	for (size_t i = 0; i < argc; ++i)
	{
		switch (argarray[i])
		{
		case rEAX:
		case rAX:
		case rAL:
		case rEBX:
		case rBX:
		case rBL:
		case rECX:
		case rCX:
		case rCL:
		case rEDX:
		case rDX:
		case rDL:
		case rESI:
		case rSI:
		case rEDI:
		case rDI:
		case rEBP:
		case rBP:
			++memsz;
			break;
		case rAH:
			break;
		case rBH:
			break;
		case rCH:
			break;
		case rDH:
			break;
		case stack1:
		case stack2:
		case stack4:
			memsz += 4;
			++stackcnt;
			break;
		}
	}
	memsz += 5; // call
	switch (ret)
	{
	case rEBX:
	case rBX:
	case rBL:
	case rECX:
	case rCX:
	case rCL:
	case rEDX:
	case rDX:
	case rDL:
	case rESI:
	case rSI:
	case rEDI:
	case rDI:
	case rEBP:
	case rBP:
		memsz += 2;
		break;
	case rAH:
		break;
	case rBH:
		break;
	case rCH:
		break;
	case rDH:
		break;
	}
	for (int i = 0; i < argc; ++i)
	{
		if (regsequal(argarray[i], ret))
			memsz += 3;
		else
			switch (argarray[i])
			{
			case rEAX:
			case rAX:
			case rAL:
			case rAH:
			case rEBX:
			case rBX:
			case rBL:
			case rBH:
			case rECX:
			case rCX:
			case rCL:
			case rCH:
			case rEDX:
			case rDX:
			case rDL:
			case rDH:
			case rESI:
			case rSI:
			case rEDI:
			case rDI:
			case rEBP:
			case rBP:
				++memsz;
				break;
			}
	}
	if (stackcnt > 0)
		memsz += 3;
	++memsz; // retn
	auto codeData = AllocateFunction(memsz);
	int cdoff = 0;
	uint8_t stackoff = stackcnt * 4;
	for (int i = argc - 1; i >= 0; --i)
	{
		switch (argarray[i])
		{
		case rEAX:
		case rAX:
		case rAL:
			codeData[cdoff++] = 0x50;
			break;
		case rEBX:
		case rBX:
		case rBL:
			codeData[cdoff++] = 0x53;
			break;
		case rECX:
		case rCX:
		case rCL:
			codeData[cdoff++] = 0x51;
			break;
		case rEDX:
		case rDX:
		case rDL:
			codeData[cdoff++] = 0x52;
			break;
		case rESI:
		case rSI:
			codeData[cdoff++] = 0x56;
			break;
		case rEDI:
		case rDI:
			codeData[cdoff++] = 0x57;
			break;
		case rEBP:
		case rBP:
			codeData[cdoff++] = 0x55;
			break;
		case rAH:
			break;
		case rBH:
			break;
		case rCH:
			break;
		case rDH:
			break;
		case stack1:
		case stack2:
		case stack4:
			writebytes(codeData, cdoff, 0xFF, 0x74, 0x24, stackoff);
			break;
		}
	}
	WriteCall(&codeData[cdoff], func);
	cdoff += 5;
	switch (ret)
	{
	case rEBX:
	case rBX:
	case rBL:
		writebytes(codeData, cdoff, 0x89, 0xC3);
		break;
	case rECX:
	case rCX:
	case rCL:
		writebytes(codeData, cdoff, 0x89, 0xC1);
		break;
	case rEDX:
	case rDX:
	case rDL:
		writebytes(codeData, cdoff, 0x89, 0xC2);
		break;
	case rESI:
	case rSI:
		writebytes(codeData, cdoff, 0x89, 0xC6);
		break;
	case rEDI:
	case rDI:
		writebytes(codeData, cdoff, 0x89, 0xC7);
		break;
	case rEBP:
	case rBP:
		writebytes(codeData, cdoff, 0x89, 0xC5);
		break;
	case rAH:
		break;
	case rBH:
		break;
	case rCH:
		break;
	case rDH:
		break;
	}
	for (int i = 0; i < argc; ++i)
	{
		if (regsequal(argarray[i], ret))
			writebytes(codeData, cdoff, 0x83, 0xC4, 4);
		else
			switch (argarray[i])
			{
			case rEAX:
			case rAX:
			case rAL:
			case rAH:
				codeData[cdoff++] = 0x58;
				break;
			case rEBX:
			case rBX:
			case rBL:
			case rBH:
				codeData[cdoff++] = 0x5B;
				break;
			case rECX:
			case rCX:
			case rCL:
			case rCH:
				codeData[cdoff++] = 0x59;
				break;
			case rEDX:
			case rDX:
			case rDL:
			case rDH:
				codeData[cdoff++] = 0x5A;
				break;
			case rESI:
			case rSI:
				codeData[cdoff++] = 0x5E;
				break;
			case rEDI:
			case rDI:
				codeData[cdoff++] = 0x5F;
				break;
			case rEBP:
			case rBP:
				codeData[cdoff++] = 0x5D;
				break;
			}
	}
	if (stackcnt > 0)
		writebytes(codeData, cdoff, 0x83, 0xC4, (uint8_t)(stackcnt * 4));
	codeData[cdoff++] = 0xC3;
#if USERCALLDEBUG
	char fn[MAX_PATH];
	sprintf_s(fn, "usercallhook@%08X.bin", address);
	auto fh = fopen(fn, "wb");
	fwrite(codeData, memsz, 1, fh);
	fclose(fh);
#endif
	assert(cdoff == memsz);
	if (*(uint8_t*)address == 0xE8)
		WriteCall((void*)address, codeData);
	else
		WriteJump((void*)address, codeData);
	free(argarray);
}

void* GenerateUserpurgeWrapperC(int ret, intptr_t address, int nbreg, ...)
{
	const size_t argc = nbreg;
	int8_t* argarray = (int8_t*)malloc(argc);
	if (!argarray)
		return nullptr;

	va_list(ap);
	va_start(ap, nbreg);
	for (int i = 0; i < nbreg; ++i)
		argarray[i] = va_arg(ap, int);
	va_end(ap);

	int stackcnt = 0;
	int memsz = 0;
	for (size_t i = 0; i < argc; ++i)
	{
		switch (argarray[i])
		{
		case rEAX:
		case rECX:
		case rEDX:
			memsz += 4;
			break;
		case rEBX:
		case rESI:
		case rEDI:
		case rEBP:
			memsz += 6;
			break;
		case rAX:
		case rCX:
		case rDX:
			memsz += 5;
			break;
		case rBX:
		case rSI:
		case rDI:
		case rBP:
			memsz += 7;
			break;
		case rAL:
		case rCL:
		case rDL:
		case rAH:
		case rCH:
		case rDH:
			memsz += 4;
			break;
		case rBL:
		case rBH:
			memsz += 6;
			break;
		case stack1:
		case stack2:
			memsz += 6;
			++stackcnt;
			break;
		case stack4:
			memsz += 4;
			++stackcnt;
			break;
		}
	}
	memsz += 5; // call
	switch (ret)
	{
	case rEBX:
	case rECX:
	case rEDX:
	case rESI:
	case rEDI:
	case rEBP:
		memsz += 2;
		break;
	case rAX:
	case rBX:
	case rCX:
	case rDX:
	case rSI:
	case rDI:
	case rBP:
	case rAL:
	case rBL:
	case rCL:
	case rDL:
	case rAH:
	case rBH:
	case rCH:
	case rDH:
		memsz += 3;
		break;
	case rst0:
		memsz += 8;
		break;
	}
	++memsz; // retn
	auto codeData = AllocateFunction(memsz);
	int cdoff = 0;
	uint8_t stackoff = argc * 4;
	for (size_t i = 0; i < argc; ++i)
	{
		switch (argarray[i])
		{
		case rEBX:
		case rBX:
		case rBH:
		case rBL:
			codeData[cdoff++] = 0x53;
			stackoff += 4;
			break;
		case rESI:
		case rSI:
			codeData[cdoff++] = 0x56;
			stackoff += 4;
			break;
		case rEDI:
		case rDI:
			codeData[cdoff++] = 0x57;
			stackoff += 4;
			break;
		case rEBP:
		case rBP:
			codeData[cdoff++] = 0x55;
			stackoff += 4;
			break;
		}
	}
	for (int i = argc - 1; i >= 0; --i)
	{
		switch (argarray[i])
		{
		case rEAX:
			writebytes(codeData, cdoff, 0x8B, 0x44, 0x24, stackoff);
			stackoff -= 4;
			break;
		case rEBX:
			writebytes(codeData, cdoff, 0x8B, 0x5C, 0x24, stackoff);
			stackoff -= 4;
			break;
		case rECX:
			writebytes(codeData, cdoff, 0x8B, 0x4C, 0x24, stackoff);
			stackoff -= 4;
			break;
		case rEDX:
			writebytes(codeData, cdoff, 0x8B, 0x54, 0x24, stackoff);
			stackoff -= 4;
			break;
		case rESI:
			writebytes(codeData, cdoff, 0x8B, 0x74, 0x24, stackoff);
			stackoff -= 4;
			break;
		case rEDI:
			writebytes(codeData, cdoff, 0x8B, 0x7C, 0x24, stackoff);
			stackoff -= 4;
			break;
		case rEBP:
			writebytes(codeData, cdoff, 0x8B, 0x6C, 0x24, stackoff);
			stackoff -= 4;
			break;
		case rAX:
			writebytes(codeData, cdoff, 0x66, 0x8B, 0x44, 0x24, stackoff);
			stackoff -= 4;
			break;
		case rBX:
			writebytes(codeData, cdoff, 0x66, 0x8B, 0x5C, 0x24, stackoff);
			stackoff -= 4;
			break;
		case rCX:
			writebytes(codeData, cdoff, 0x66, 0x8B, 0x4C, 0x24, stackoff);
			stackoff -= 4;
			break;
		case rDX:
			writebytes(codeData, cdoff, 0x66, 0x8B, 0x54, 0x24, stackoff);
			stackoff -= 4;
			break;
		case rSI:
			writebytes(codeData, cdoff, 0x66, 0x8B, 0x74, 0x24, stackoff);
			stackoff -= 4;
			break;
		case rDI:
			writebytes(codeData, cdoff, 0x66, 0x8B, 0x7C, 0x24, stackoff);
			stackoff -= 4;
			break;
		case rBP:
			writebytes(codeData, cdoff, 0x66, 0x8B, 0x6C, 0x24, stackoff);
			stackoff -= 4;
			break;
		case rAL:
			writebytes(codeData, cdoff, 0x8A, 0x44, 0x24, stackoff);
			stackoff -= 4;
			break;
		case rBL:
			writebytes(codeData, cdoff, 0x8A, 0x5C, 0x24, stackoff);
			stackoff -= 4;
			break;
		case rCL:
			writebytes(codeData, cdoff, 0x8A, 0x4C, 0x24, stackoff);
			stackoff -= 4;
			break;
		case rDL:
			writebytes(codeData, cdoff, 0x8A, 0x54, 0x24, stackoff);
			stackoff -= 4;
			break;
		case rAH:
			writebytes(codeData, cdoff, 0x8A, 0x64, 0x24, stackoff);
			stackoff -= 4;
			break;
		case rBH:
			writebytes(codeData, cdoff, 0x8A, 0x7C, 0x24, stackoff);
			stackoff -= 4;
			break;
		case rCH:
			writebytes(codeData, cdoff, 0x8A, 0x6C, 0x24, stackoff);
			stackoff -= 4;
			break;
		case rDH:
			writebytes(codeData, cdoff, 0x8A, 0x74, 0x24, stackoff);
			stackoff -= 4;
			break;
		case stack1:
			writebytes(codeData, cdoff, 0x0F, 0xBE, 0x44, 0x24, stackoff, 0x50);
			break;
		case stack2:
			writebytes(codeData, cdoff, 0x0F, 0xBF, 0x44, 0x24, stackoff, 0x50);
			break;
		case stack4:
			writebytes(codeData, cdoff, 0xFF, 0x74, 0x24, stackoff);
			break;
		}
	}
	WriteCall(&codeData[cdoff], (void*)address);
	cdoff += 5;
	for (int i = argc - 1; i >= 0; --i)
	{
		switch (argarray[i])
		{
		case rEBX:
		case rBX:
		case rBL:
		case rBH:
			codeData[cdoff++] = 0x5B;
			break;
		case rESI:
		case rSI:
			codeData[cdoff++] = 0x5E;
			break;
		case rEDI:
		case rDI:
			codeData[cdoff++] = 0x5F;
			break;
		case rEBP:
		case rBP:
			codeData[cdoff++] = 0x5D;
			break;
		}
	}
	switch (ret)
	{
	case rEBX:
		writebytes(codeData, cdoff, 0x89, 0xD8);
		break;
	case rECX:
		writebytes(codeData, cdoff, 0x89, 0xC8);
		break;
	case rEDX:
		writebytes(codeData, cdoff, 0x89, 0xD0);
		break;
	case rESI:
		writebytes(codeData, cdoff, 0x89, 0xF0);
		break;
	case rEDI:
		writebytes(codeData, cdoff, 0x89, 0xF8);
		break;
	case rEBP:
		writebytes(codeData, cdoff, 0x89, 0xE8);
		break;
	case rAX:
		writebytes(codeData, cdoff, 0x0F, 0xBF, 0xC0);
		break;
	case rBX:
		writebytes(codeData, cdoff, 0x0F, 0xBF, 0xC3);
		break;
	case rCX:
		writebytes(codeData, cdoff, 0x0F, 0xBF, 0xC1);
		break;
	case rDX:
		writebytes(codeData, cdoff, 0x0F, 0xBF, 0xC2);
		break;
	case rSI:
		writebytes(codeData, cdoff, 0x0F, 0xBF, 0xC6);
		break;
	case rDI:
		writebytes(codeData, cdoff, 0x0F, 0xBF, 0xC7);
		break;
	case rBP:
		writebytes(codeData, cdoff, 0x0F, 0xBF, 0xC5);
		break;
	case rAL:
		writebytes(codeData, cdoff, 0x0F, 0xBE, 0xC0);
		break;
	case rBL:
		writebytes(codeData, cdoff, 0x0F, 0xBE, 0xC3);
		break;
	case rCL:
		writebytes(codeData, cdoff, 0x0F, 0xBE, 0xC1);
		break;
	case rDL:
		writebytes(codeData, cdoff, 0x0F, 0xBE, 0xC2);
		break;
	case rAH:
		writebytes(codeData, cdoff, 0x0F, 0xBE, 0xC4);
		break;
	case rBH:
		writebytes(codeData, cdoff, 0x0F, 0xBE, 0xC7);
		break;
	case rCH:
		writebytes(codeData, cdoff, 0x0F, 0xBE, 0xC5);
		break;
	case rDH:
		writebytes(codeData, cdoff, 0x0F, 0xBE, 0xC6);
		break;
	case rst0:
		writebytes(codeData, cdoff, 0xD9, 0x5C, 0x24, 0xFC, 0x8B, 0x44, 0x24, 0xFC);
		break;
	}
	codeData[cdoff++] = 0xC3;
#if USERCALLDEBUG
	char fn[MAX_PATH];
	sprintf_s(fn, "usercallwrapper@%08X.bin", address);
	auto fh = fopen(fn, "wb");
	fwrite(codeData, memsz, 1, fh);
	fclose(fh);
#endif
	assert(cdoff == memsz);

	free(argarray);
	return codeData;
}

void GenerateUserpurgeHookC(void* func, int ret, intptr_t address, int nbreg, ...)
{
	const size_t argc = nbreg;
	int8_t* argarray = (int8_t*)malloc(argc);
	if (!argarray)
		return;

	va_list(ap);
	va_start(ap, nbreg);
	for (int i = 0; i < nbreg; ++i)
		argarray[i] = va_arg(ap, int);
	va_end(ap);

	int stackcnt = 0;
	int memsz = 0;
	for (size_t i = 0; i < argc; ++i)
	{
		switch (argarray[i])
		{
		case rEAX:
		case rAX:
		case rAL:
		case rEBX:
		case rBX:
		case rBL:
		case rECX:
		case rCX:
		case rCL:
		case rEDX:
		case rDX:
		case rDL:
		case rESI:
		case rSI:
		case rEDI:
		case rDI:
		case rEBP:
		case rBP:
			++memsz;
			break;
		case rAH:
			break;
		case rBH:
			break;
		case rCH:
			break;
		case rDH:
			break;
		case stack1:
		case stack2:
		case stack4:
			memsz += 4;
			++stackcnt;
			break;
		}
	}
	memsz += 5; // call
	switch (ret)
	{
	case rEBX:
	case rBX:
	case rBL:
	case rECX:
	case rCX:
	case rCL:
	case rEDX:
	case rDX:
	case rDL:
	case rESI:
	case rSI:
	case rEDI:
	case rDI:
	case rEBP:
	case rBP:
		memsz += 2;
		break;
	case rAH:
		break;
	case rBH:
		break;
	case rCH:
		break;
	case rDH:
		break;
	}
	for (int i = 0; i < argc; ++i)
	{
		if (regsequal(argarray[i], ret))
			memsz += 3;
		else
			switch (argarray[i])
			{
			case rEAX:
			case rAX:
			case rAL:
			case rAH:
			case rEBX:
			case rBX:
			case rBL:
			case rBH:
			case rECX:
			case rCX:
			case rCL:
			case rCH:
			case rEDX:
			case rDX:
			case rDL:
			case rDH:
			case rESI:
			case rSI:
			case rEDI:
			case rDI:
			case rEBP:
			case rBP:
				++memsz;
				break;
			}
	}
	if (stackcnt > 0)
		memsz += 3 + 2;
	++memsz; // retn
	auto codeData = AllocateFunction(memsz);
	int cdoff = 0;
	uint8_t stackoff = stackcnt * 4;
	for (int i = argc - 1; i >= 0; --i)
	{
		switch (argarray[i])
		{
		case rEAX:
		case rAX:
		case rAL:
			codeData[cdoff++] = 0x50;
			break;
		case rEBX:
		case rBX:
		case rBL:
			codeData[cdoff++] = 0x53;
			break;
		case rECX:
		case rCX:
		case rCL:
			codeData[cdoff++] = 0x51;
			break;
		case rEDX:
		case rDX:
		case rDL:
			codeData[cdoff++] = 0x52;
			break;
		case rESI:
		case rSI:
			codeData[cdoff++] = 0x56;
			break;
		case rEDI:
		case rDI:
			codeData[cdoff++] = 0x57;
			break;
		case rEBP:
		case rBP:
			codeData[cdoff++] = 0x55;
			break;
		case rAH:
			break;
		case rBH:
			break;
		case rCH:
			break;
		case rDH:
			break;
		case stack1:
		case stack2:
		case stack4:
			writebytes(codeData, cdoff, 0xFF, 0x74, 0x24, stackoff);
			break;
		}
	}
	WriteCall(&codeData[cdoff], func);
	cdoff += 5;
	switch (ret)
	{
	case rEBX:
	case rBX:
	case rBL:
		writebytes(codeData, cdoff, 0x89, 0xC3);
		break;
	case rECX:
	case rCX:
	case rCL:
		writebytes(codeData, cdoff, 0x89, 0xC1);
		break;
	case rEDX:
	case rDX:
	case rDL:
		writebytes(codeData, cdoff, 0x89, 0xC2);
		break;
	case rESI:
	case rSI:
		writebytes(codeData, cdoff, 0x89, 0xC6);
		break;
	case rEDI:
	case rDI:
		writebytes(codeData, cdoff, 0x89, 0xC7);
		break;
	case rEBP:
	case rBP:
		writebytes(codeData, cdoff, 0x89, 0xC5);
		break;
	case rAH:
		break;
	case rBH:
		break;
	case rCH:
		break;
	case rDH:
		break;
	}
	for (int i = 0; i < argc; ++i)
	{
		if (regsequal(argarray[i], ret))
			writebytes(codeData, cdoff, 0x83, 0xC4, 4);
		else
			switch (argarray[i])
			{
			case rEAX:
			case rAX:
			case rAL:
			case rAH:
				codeData[cdoff++] = 0x58;
				break;
			case rEBX:
			case rBX:
			case rBL:
			case rBH:
				codeData[cdoff++] = 0x5B;
				break;
			case rECX:
			case rCX:
			case rCL:
			case rCH:
				codeData[cdoff++] = 0x59;
				break;
			case rEDX:
			case rDX:
			case rDL:
			case rDH:
				codeData[cdoff++] = 0x5A;
				break;
			case rESI:
			case rSI:
				codeData[cdoff++] = 0x5E;
				break;
			case rEDI:
			case rDI:
				codeData[cdoff++] = 0x5F;
				break;
			case rEBP:
			case rBP:
				codeData[cdoff++] = 0x5D;
				break;
			}
	}
	if (stackcnt > 0) {
		writebytes(codeData, cdoff, 0x83, 0xC4, (uint8_t)(stackcnt * 4));
		writebytes(codeData, cdoff, 0xC2, (uint8_t)(stackcnt * 4), 0x00); // retn + callee cleanup
	}
	else
		codeData[cdoff++] = 0xC3;
#if USERCALLDEBUG
	char fn[MAX_PATH];
	sprintf_s(fn, "usercallhook@%08X.bin", address);
	auto fh = fopen(fn, "wb");
	fwrite(codeData, memsz, 1, fh);
	fclose(fh);
#endif
	assert(cdoff == memsz);
	if (*(uint8_t*)address == 0xE8)
		WriteCall((void*)address, codeData);
	else
		WriteJump((void*)address, codeData);
	free(argarray);
}