#pragma once

/*
This is a faster version of the FunctionHook classes from the SADX Mod Loader utilities.

While FunctionHook's method is very minimalist and safe, it requires memory reading and writing for every call to original.
This uses the same principle as Trampoline from the same utilities or Minhook/Detour, but it's wrapped in easy to use classes.

There are two main versions: FastFunctionHook (and its derivatives) and FastFunctionHookPtr.
The former requires you to type the return and argument types manually and the latter requires the function pointer.

Planned:
- Support conditional jumps in trampoline.
- Add an optional relay function before the hook function.

Contributions:
- MainMemory (original code, usercal wrappers, etc.)
- Kell (fast version)
- Vyacheslav Patkov (disassembler)
*/

#include <exception>
#include <cstdint>

#include "MemAccess.h"
#include "UsercallFunctionHandler.h"

void* GenerateTrampoline(uint8_t* origbytes);
void* GenerateThiscallWrapper(void* address, int stackcnt);
void GenerateThiscallHook(void* func, void* address, int stackcnt);
void* GenerateUsercallWrapperC(int ret, intptr_t address, int nbreg, ...);
void GenerateUsercallHookC(void* func, int ret, intptr_t address, int nbreg, ...);
void* GenerateUserpurgeWrapperC(int ret, intptr_t address, int nbreg, ...);
void GenerateUserpurgeHookC(void* func, int ret, intptr_t address, int nbreg, ...);

/// <summary>
/// A function hooking class using the function return and argument types as a base.
/// </summary>
/// <typeparam name="TRet">The return type.</typeparam>
/// <typeparam name="...TArgs">The type of each argument from left to right.</typeparam>
template<typename TRet, typename... TArgs>
class FastFunctionHook
{
public:
	typedef TRet(*FuncType)(TArgs...);

	FastFunctionHook(FuncType address) : origaddr(address) {	}

	FastFunctionHook(intptr_t address) : origaddr(reinterpret_cast<FuncType>(address)) {	}

	// Initialize the object and immediately apply a hook.
	FastFunctionHook(FuncType address, FuncType hook) : FastFunctionHook(address)
	{
		Hook(hook);
	}

	// Initialize the object and immediately apply a hook.
	FastFunctionHook(intptr_t address, FuncType hook) : FastFunctionHook(address)
	{
		Hook(hook);
	}

	FastFunctionHook() = delete;
	FastFunctionHook(FastFunctionHook&) = delete;
	FastFunctionHook(FastFunctionHook&&) = delete;

	// Apply a hook to the unhooked function.
	void Hook(FuncType hook)
	{
		if (trampoline)
			throw new std::exception("Cannot apply hook to already hooked function!");
		trampoline = (FuncType)GenerateTrampoline((uint8_t*)origaddr);
		WriteJump(origaddr, hook); // Write jump from original function to the hook
	}

	// Call the original function, bypassing the hook.
	TRet Original(TArgs... args)
	{
		if (trampoline)
		{
			return trampoline(args...);
		}
		else
		{
			TRet ret = TRet();
			return ret;
		}
	}

private:
	const FuncType origaddr;
	FuncType trampoline = nullptr;
};

/// <summary>
/// A void function hooking class using the function argument types as a base.
/// </summary>
/// <typeparam name="...TArgs">The type of each argument from left to right.</typeparam>
template<typename... TArgs>
class FastFunctionHook<void, TArgs...>
{
public:
	typedef void(*FuncType)(TArgs...);

	FastFunctionHook(FuncType address) : origaddr(address) {	}

	FastFunctionHook(intptr_t address) : origaddr(reinterpret_cast<FuncType>(address)) {	}

	// Initialize the object and immediately apply a hook.
	FastFunctionHook(FuncType address, FuncType hook) : FastFunctionHook(address)
	{
		Hook(hook);
	}

	// Initialize the object and immediately apply a hook.
	FastFunctionHook(intptr_t address, FuncType hook) : FastFunctionHook(address)
	{
		Hook(hook);
	}

	FastFunctionHook() = delete;
	FastFunctionHook(FastFunctionHook&) = delete;
	FastFunctionHook(FastFunctionHook&&) = delete;

	// Apply a hook to the unhooked function.
	void Hook(FuncType hook)
	{
		if (trampoline)
			throw new std::exception("Cannot apply hook to already hooked function!");
		trampoline = (FuncType)GenerateTrampoline((uint8_t*)origaddr);
		WriteJump(origaddr, hook); // Write jump from original function to the hook
	}

	// Call the original function, bypassing the hook.
	void Original(TArgs... args)
	{
		if (trampoline)
		{
			trampoline(args...);
		}
	}

private:
	const FuncType origaddr;
	FuncType trampoline = nullptr;
};

/// <summary>
/// An stdcall function hooking class using the function return and argument types as a base.
/// </summary>
/// <typeparam name="TRet">The return type.</typeparam>
/// <typeparam name="...TArgs">The type of each argument from left to right.</typeparam>
template<typename TRet, typename... TArgs>
class FastStdcallHook
{
public:
	typedef TRet(__stdcall*FuncType)(TArgs...);

	FastStdcallHook(FuncType address) : origaddr(address) {	}

	FastStdcallHook(intptr_t address) : origaddr(reinterpret_cast<FuncType>(address)) {	}

	// Initialize the object and immediately apply a hook.
	FastStdcallHook(FuncType address, FuncType hook) : FastStdcallHook(address)
	{
		Hook(hook);
	}

	// Initialize the object and immediately apply a hook.
	FastStdcallHook(intptr_t address, FuncType hook) : FastStdcallHook(address)
	{
		Hook(hook);
	}

	FastStdcallHook() = delete;
	FastStdcallHook(FastStdcallHook&) = delete;
	FastStdcallHook(FastStdcallHook&&) = delete;

	// Apply a hook to the unhooked function.
	void Hook(FuncType hook)
	{
		if (trampoline)
			throw new std::exception("Cannot apply hook to already hooked function!");
		trampoline = (FuncType)GenerateTrampoline((uint8_t*)origaddr);
		WriteJump(origaddr, hook); // Write jump from original function to the hook
	}

	// Call the original function, bypassing the hook.
	TRet Original(TArgs... args)
	{
		if (trampoline)
		{
			return trampoline(args...);
		}
		else
		{
			TRet ret = TRet();
			return ret;
		}
	}

private:
	const FuncType origaddr;
	FuncType trampoline = nullptr;
};

/// <summary>
/// A void stdcall function hooking class using the function argument types as a base.
/// </summary>
/// <typeparam name="...TArgs">The type of each argument from left to right.</typeparam>
template<typename... TArgs>
class FastStdcallHook<void, TArgs...>
{
public:
	typedef void(__stdcall*FuncType)(TArgs...);

	FastStdcallHook(FuncType address) : origaddr(address) {	}

	FastStdcallHook(intptr_t address) : origaddr(reinterpret_cast<FuncType>(address)) {	}

	// Initialize the object and immediately apply a hook.
	FastStdcallHook(FuncType address, FuncType hook) : FastStdcallHook(address)
	{
		Hook(hook);
	}

	// Initialize the object and immediately apply a hook.
	FastStdcallHook(intptr_t address, FuncType hook) : FastStdcallHook(address)
	{
		Hook(hook);
	}

	FastStdcallHook() = delete;
	FastStdcallHook(FastStdcallHook&) = delete;
	FastStdcallHook(FastStdcallHook&&) = delete;

	// Apply a hook to the unhooked function.
	void Hook(FuncType hook)
	{
		if (trampoline)
			throw new std::exception("Cannot apply hook to already hooked function!");
		trampoline = (FuncType)GenerateTrampoline((uint8_t*)origaddr);
		WriteJump(origaddr, hook); // Write jump from original function to the hook
	}

	// Call the original function, bypassing the hook.
	void Original(TArgs... args)
	{
		if (trampoline)
		{
			trampoline(args...);
		}
	}

private:
	const FuncType origaddr;
	FuncType trampoline = nullptr;
};

/// <summary>
/// A fastcall function hooking class using the function return and argument types as a base.
/// </summary>
/// <typeparam name="TRet">The return type.</typeparam>
/// <typeparam name="...TArgs">The type of each argument from left to right.</typeparam>
template<typename TRet, typename... TArgs>
class FastFastcallHook
{
public:
	typedef TRet(__fastcall* FuncType)(TArgs...);

	FastFastcallHook(FuncType address) : origaddr(address) {	}

	FastFastcallHook(intptr_t address) : origaddr(reinterpret_cast<FuncType>(address)) {	}

	// Initialize the object and immediately apply a hook.
	FastFastcallHook(FuncType address, FuncType hook) : FastFastcallHook(address)
	{
		Hook(hook);
	}

	// Initialize the object and immediately apply a hook.
	FastFastcallHook(intptr_t address, FuncType hook) : FastFastcallHook(address)
	{
		Hook(hook);
	}

	FastFastcallHook() = delete;
	FastFastcallHook(FastFastcallHook&) = delete;
	FastFastcallHook(FastFastcallHook&&) = delete;

	// Apply a hook to the unhooked function.
	void Hook(FuncType hook)
	{
		if (trampoline)
			throw new std::exception("Cannot apply hook to already hooked function!");
		trampoline = (FuncType)GenerateTrampoline((uint8_t*)origaddr);
		WriteJump(origaddr, hook); // Write jump from original function to the hook
	}

	// Call the original function, bypassing the hook.
	TRet Original(TArgs... args)
	{
		if (trampoline)
		{
			return trampoline(args...);
		}
		else
		{
			TRet ret = TRet();
			return ret;
		}
	}

private:
	const FuncType origaddr;
	FuncType trampoline = nullptr;
};

/// <summary>
/// A void fastcall function hooking class using the function argument types as a base.
/// </summary>
/// <typeparam name="...TArgs">The type of each argument from left to right.</typeparam>
template<typename... TArgs>
class FastFastcallHook<void, TArgs...>
{
public:
	typedef void(__fastcall* FuncType)(TArgs...);

	FastFastcallHook(FuncType address) : origaddr(address) {	}

	FastFastcallHook(intptr_t address) : origaddr(reinterpret_cast<FuncType>(address)) {	}

	// Initialize the object and immediately apply a hook.
	FastFastcallHook(FuncType address, FuncType hook) : FastFastcallHook(address)
	{
		Hook(hook);
	}

	// Initialize the object and immediately apply a hook.
	FastFastcallHook(intptr_t address, FuncType hook) : FastFastcallHook(address)
	{
		Hook(hook);
	}

	FastFastcallHook() = delete;
	FastFastcallHook(FastFastcallHook&) = delete;
	FastFastcallHook(FastFastcallHook&&) = delete;

	// Apply a hook to the unhooked function.
	void Hook(FuncType hook)
	{
		if (trampoline)
			throw new std::exception("Cannot apply hook to already hooked function!");
		trampoline = (FuncType)GenerateTrampoline((uint8_t*)origaddr);
		WriteJump(origaddr, hook); // Write jump from original function to the hook
	}

	// Call the original function, bypassing the hook.
	void Original(TArgs... args)
	{
		if (trampoline)
		{
			trampoline(args...);
		}
	}

private:
	const FuncType origaddr;
	FuncType trampoline = nullptr;
};

/// <summary>
/// A thiscall function hooking class using the function return and argument types as a base.
/// The hooked function should be cdecl with the this pointer as the first argument.
/// </summary>
/// <typeparam name="TRet">The return type.</typeparam>
/// <typeparam name="...TArgs">The type of each argument from left to right.</typeparam>
template<typename TRet, typename... TArgs>
class FastThiscallHook
{
public:
	typedef TRet(*FuncType)(TArgs...);

	FastThiscallHook(FuncType address) : origaddr(address) {   }

	FastThiscallHook(intptr_t address) : origaddr(reinterpret_cast<FuncType>(address)) {	}

	// Initialize the object and immediately apply a hook.
	FastThiscallHook(FuncType address, FuncType hook) : FastThiscallHook(address)
	{
		Hook(hook);
	}

	// Initialize the object and immediately apply a hook.
	FastThiscallHook(intptr_t address, FuncType hook) : FastThiscallHook(address)
	{
		Hook(hook);
	}

	FastThiscallHook() = delete;
	FastThiscallHook(FastThiscallHook&) = delete;
	FastThiscallHook(FastThiscallHook&&) = delete;

	// Apply a hook to the unhooked function.
	void Hook(FuncType hook)
	{
		if (trampoline)
			throw new std::exception("Cannot apply hook to already hooked function!");
		trampoline = (FuncType)GenerateThiscallWrapper(GenerateTrampoline((uint8_t*)origaddr), sizeof...(TArgs) - 1);
		GenerateThiscallHook(hook, origaddr, sizeof...(TArgs) - 1);
	}

	// Call the original function, bypassing the hook.
	TRet Original(TArgs... args)
	{
		if (trampoline)
		{
			return trampoline(args...);
		}
		else
		{
			TRet ret = TRet();
			return ret;
		}
	}

private:
	const FuncType origaddr;
	FuncType trampoline = nullptr;
};

/// <summary>
/// A void function hooking class using the function argument types as a base.
/// The hooked function should be cdecl with the this pointer as the first argument.
/// </summary>
/// <typeparam name="...TArgs">The type of each argument from left to right.</typeparam>
template<typename... TArgs>
class FastThiscallHook<void, TArgs...>
{
public:
	typedef void(*FuncType)(TArgs...);

	FastThiscallHook(FuncType address) : origaddr(address) {   }

	FastThiscallHook(intptr_t address) : origaddr(reinterpret_cast<FuncType>(address)) {	}

	// Initialize the object and immediately apply a hook.
	FastThiscallHook(FuncType address, FuncType hook) : FastThiscallHook(address)
	{
		Hook(hook);
	}

	// Initialize the object and immediately apply a hook.
	FastThiscallHook(intptr_t address, FuncType hook) : FastThiscallHook(address)
	{
		Hook(hook);
	}

	FastThiscallHook() = delete;
	FastThiscallHook(FastThiscallHook&) = delete;
	FastThiscallHook(FastThiscallHook&&) = delete;

	// Apply a hook to the unhooked function.
	void Hook(FuncType hook)
	{
		if (trampoline)
			throw new std::exception("Cannot apply hook to already hooked function!");
		trampoline = (FuncType)GenerateThiscallWrapper(GenerateTrampoline((uint8_t*)origaddr), sizeof...(TArgs) - 1);
		GenerateThiscallHook(hook, origaddr, sizeof...(TArgs) - 1);
	}

	// Call the original function, bypassing the hook.
	void Original(TArgs... args)
	{
		if (trampoline)
		{
			return trampoline(args...);
		}
	}

private:
	const FuncType origaddr;
	FuncType trampoline = nullptr;
};

/// <summary>
/// A function hooking class using the function return and argument types as a base for non-standard calling conventions.
/// </summary>
/// <typeparam name="TRet">The return type.</typeparam>
/// <typeparam name="...TArgs">The type of each argument from left to right.</typeparam>
template<typename TRet, typename... TArgs>
class FastUsercallHook
{
public:
	typedef TRet(*FuncType)(TArgs...);

	FastUsercallHook(FuncType address) : origaddr(address) {   }

	FastUsercallHook(intptr_t address) : origaddr(reinterpret_cast<FuncType>(address)) {	}
	
	// Initialize the object and immediately apply a hook.
	template<typename... TRegs>
	FastUsercallHook(FuncType address, FuncType hook, int ret, TRegs... regs) : FastUsercallHook(address)
	{
		Hook(hook, ret, regs...);
	}
	
	// Initialize the object and immediately apply a hook.
	template<typename... TRegs>
	FastUsercallHook(intptr_t address, FuncType hook, int ret, TRegs... regs) : FastUsercallHook(address)
	{
		Hook(hook, ret, regs...);
	}

	FastUsercallHook() = delete;
	FastUsercallHook(FastUsercallHook&) = delete;
	FastUsercallHook(FastUsercallHook&&) = delete;

	// Apply a hook to the unhooked function.
	template<typename... TRegs>
	void Hook(FuncType hook, int ret, TRegs... regs)
	{
		if (trampoline)
			throw new std::exception("Cannot apply hook to already hooked function!");
		trampoline = (FuncType)GenerateUsercallWrapperC(ret, (intptr_t)GenerateTrampoline((uint8_t*)origaddr), sizeof...(regs), regs...);
		GenerateUsercallHookC(hook, ret, (intptr_t)origaddr, sizeof...(regs), regs...);
	}

	// Call the original function, bypassing the hook.
	TRet Original(TArgs... args)
	{
		if (trampoline)
		{
			return trampoline(args...);
		}
		else
		{
			TRet ret = TRet();
			return ret;
		}
	}

private:
	const FuncType origaddr;
	FuncType trampoline = nullptr;
};

/// <summary>
/// A void function hooking class using the function argument types as a base for non-standard calling conventions.
/// </summary>
/// <typeparam name="...TArgs">The type of each argument from left to right.</typeparam>
template<typename... TArgs>
class FastUsercallHook<void, TArgs...>
{
public:
	typedef void(*FuncType)(TArgs...);

	FastUsercallHook(FuncType address) : origaddr(address) {   }

	FastUsercallHook(intptr_t address) : origaddr(reinterpret_cast<FuncType>(address)) {	}

	// Initialize the object and immediately apply a hook.
	template<typename... TRegs>
	FastUsercallHook(FuncType address, FuncType hook, int ret, TRegs... regs) : FastUsercallHook(address)
	{
		Hook(hook, ret, regs...);
	}

	// Initialize the object and immediately apply a hook.
	template<typename... TRegs>
	FastUsercallHook(intptr_t address, FuncType hook, int ret, TRegs... regs) : FastUsercallHook(address)
	{
		Hook(hook, ret, regs...);
	}

	FastUsercallHook() = delete;
	FastUsercallHook(FastUsercallHook&) = delete;
	FastUsercallHook(FastUsercallHook&&) = delete;

	// Apply a hook to the unhooked function.
	template<typename... TRegs>
	void Hook(FuncType hook, TRegs... regs)
	{
		if (trampoline)
			throw new std::exception("Cannot apply hook to already hooked function!");
		trampoline = (FuncType)GenerateUsercallWrapperC(noret, (intptr_t)GenerateTrampoline((uint8_t*)origaddr), sizeof...(regs), regs...);
		GenerateUsercallHookC(hook, noret, (intptr_t)origaddr, sizeof...(regs), regs...);
	}

	// Call the original function, bypassing the hook.
	void Original(TArgs... args)
	{
		if (trampoline)
		{
			return trampoline(args...);
		}
	}

private:
	const FuncType origaddr;
	FuncType trampoline = nullptr;
};

/// <summary>
/// A function hooking class using the function return and argument types as a base for non-standard calling conventions with callee cleanup.
/// </summary>
/// <typeparam name="TRet">The return type.</typeparam>
/// <typeparam name="...TArgs">The type of each argument from left to right.</typeparam>
template<typename TRet, typename... TArgs>
class FastUserpurgeHook
{
public:
	typedef TRet(*FuncType)(TArgs...);

	FastUserpurgeHook(FuncType address) : origaddr(address) {   }

	FastUserpurgeHook(intptr_t address) : origaddr(reinterpret_cast<FuncType>(address)) {	}
	
	// Initialize the object and immediately apply a hook.
	template<typename... TRegs>
	FastUserpurgeHook(FuncType address, FuncType hook, int ret, TRegs... regs) : FastUserpurgeHook(address)
	{
		Hook(hook, ret, regs...);
	}
	
	// Initialize the object and immediately apply a hook.
	template<typename... TRegs>
	FastUserpurgeHook(intptr_t address, FuncType hook, int ret, TRegs... regs) : FastUserpurgeHook(address)
	{
		Hook(hook, ret, regs...);
	}

	FastUserpurgeHook() = delete;
	FastUserpurgeHook(FastUserpurgeHook&) = delete;
	FastUserpurgeHook(FastUserpurgeHook&&) = delete;

	// Apply a hook to the unhooked function.
	template<typename... TRegs>
	void Hook(FuncType hook, int ret, TRegs... regs)
	{
		if (trampoline)
			throw new std::exception("Cannot apply hook to already hooked function!");
		trampoline = (FuncType)GenerateUserpurgeWrapperC(ret, (intptr_t)GenerateTrampoline((uint8_t*)origaddr), sizeof...(regs), regs...);
		GenerateUserpurgeHookC(hook, ret, (intptr_t)origaddr, sizeof...(regs), regs...);
	}

	// Call the original function, bypassing the hook.
	TRet Original(TArgs... args)
	{
		if (trampoline)
		{
			return trampoline(args...);
		}
		else
		{
			TRet ret = TRet();
			return ret;
		}
	}

private:
	const FuncType origaddr;
	FuncType trampoline = nullptr;
};

/// <summary>
/// A void function hooking class using the function argument types as a base for non-standard calling conventions with callee cleanup.
/// </summary>
/// <typeparam name="...TArgs">The type of each argument from left to right.</typeparam>
template<typename... TArgs>
class FastUserpurgeHook<void, TArgs...>
{
public:
	typedef void(*FuncType)(TArgs...);

	FastUserpurgeHook(FuncType address) : origaddr(address) {   }

	FastUserpurgeHook(intptr_t address) : origaddr(reinterpret_cast<FuncType>(address)) {	}

	// Initialize the object and immediately apply a hook.
	template<typename... TRegs>
	FastUserpurgeHook(FuncType address, FuncType hook, int ret, TRegs... regs) : FastUserpurgeHook(address)
	{
		Hook(hook, ret, regs...);
	}

	// Initialize the object and immediately apply a hook.
	template<typename... TRegs>
	FastUserpurgeHook(intptr_t address, FuncType hook, int ret, TRegs... regs) : FastUserpurgeHook(address)
	{
		Hook(hook, ret, regs...);
	}

	FastUserpurgeHook() = delete;
	FastUserpurgeHook(FastUserpurgeHook&) = delete;
	FastUserpurgeHook(FastUserpurgeHook&&) = delete;

	// Apply a hook to the unhooked function.
	template<typename... TRegs>
	void Hook(FuncType hook, TRegs... regs)
	{
		if (trampoline)
			throw new std::exception("Cannot apply hook to already hooked function!");
		trampoline = (FuncType)GenerateUserpurgeWrapperC(noret, (intptr_t)GenerateTrampoline((uint8_t*)origaddr), sizeof...(regs), regs...);
		GenerateUserpurgeHookC(hook, noret, (intptr_t)origaddr, sizeof...(regs), regs...);
	}

	// Call the original function, bypassing the hook.
	void Original(TArgs... args)
	{
		if (trampoline)
		{
			return trampoline(args...);
		}
	}

private:
	const FuncType origaddr;
	FuncType trampoline = nullptr;
};

/// <summary>
/// A function hooking class using a function pointer as a base.
/// Note: Original is a function pointer and may be null if Hook was not called before.
/// </summary>
/// <typeparam name="FuncType">Function pointer defining the hooked function.</typeparam>
template<typename FuncType>
class FastFunctionHookPtr
{
public:
	FastFunctionHookPtr(FuncType address) : origaddr(address) {	}

	FastFunctionHookPtr(intptr_t address) : origaddr(reinterpret_cast<FuncType>(address)) {	}

	// Initialize the object and immediately apply a hook.
	FastFunctionHookPtr(FuncType address, FuncType hook) : FastFunctionHookPtr(address)
	{
		Hook(hook);
	}

	// Initialize the object and immediately apply a hook.
	FastFunctionHookPtr(intptr_t address, FuncType hook) : FastFunctionHookPtr(address)
	{
		Hook(hook);
	}

	FastFunctionHookPtr() = delete;
	FastFunctionHookPtr(FastFunctionHookPtr&) = delete;
	FastFunctionHookPtr(FastFunctionHookPtr&&) = delete;

	// Apply a hook to the unhooked function.
	void Hook(FuncType hook)
	{
		if (Original)
			throw new std::exception("Cannot apply hook to already hooked function!");
		Original = (FuncType)GenerateTrampoline((uint8_t*)origaddr);
		WriteJump(origaddr, hook); // Write jump from original function to the hook
	}

	// Call the original function, bypassing the hook.
	FuncType Original = nullptr;

private:
	const FuncType origaddr;
};

/// <summary>
/// A function hooking class using a function pointer as a base for non-standard calling conventions.
/// Note: Original is a function pointer and may be null if Hook was not called before.
/// Use enum in UsercallFunctionHandler.h to specify each register from left to right.
/// </summary>
/// <typeparam name="FuncType">Function pointer defining the hooked function.</typeparam>
/// <typeparam name="TRetReg">The return register.</typeparam>
/// <typeparam name="TRegs">Registers of each argument in left to right order.</typeparam>
template<typename FuncType, int TRetReg, int... TRegs>
class FastUsercallHookPtr
{
public:
	FastUsercallHookPtr(FuncType address) : origaddr(address) {   }

	FastUsercallHookPtr(intptr_t address) : origaddr(reinterpret_cast<FuncType>(address)) {	}

	// Initialize the object and immediately apply a hook.
	FastUsercallHookPtr(FuncType address, FuncType hook) : FastUsercallHookPtr(address)
	{
		Hook(hook);
	}

	// Initialize the object and immediately apply a hook.
	FastUsercallHookPtr(intptr_t address, FuncType hook) : FastUsercallHookPtr(address)
	{
		Hook(hook);
	}

	FastUsercallHookPtr() = delete;
	FastUsercallHookPtr(FastUsercallHookPtr&) = delete;
	FastUsercallHookPtr(FastUsercallHookPtr&&) = delete;

	// Apply a hook to the unhooked function.
	void Hook(FuncType hook)
	{
		if (Original)
			throw new std::exception("Cannot apply hook to already hooked function!");
		Original = (FuncType)GenerateUsercallWrapperC(TRetReg, (intptr_t)GenerateTrampoline((uint8_t*)origaddr), sizeof...(TRegs), TRegs...);
		GenerateUsercallHookC(hook, TRetReg, (intptr_t)origaddr, sizeof...(TRegs), TRegs...);
	}

	// Call the original function, bypassing the hook.
	FuncType Original = nullptr;

private:
	const FuncType origaddr;
};

/// <summary>
/// A function hooking class using a function pointer as a base for non-standard calling conventions with callee cleanup.
/// Note: Original is a function pointer and may be null if Hook was not called before.
/// Use enum in UsercallFunctionHandler.h to specify each register from left to right.
/// </summary>
/// <typeparam name="FuncType">Function pointer defining the hooked function.</typeparam>
/// <typeparam name="TRetReg">The return register.</typeparam>
/// <typeparam name="TRegs">Registers of each argument in left to right order.</typeparam>
template<typename FuncType, int TRetReg, int... TRegs>
class FastUserpurgeHookPtr
{
public:
	FastUserpurgeHookPtr(FuncType address) : origaddr(address) {   }

	FastUserpurgeHookPtr(intptr_t address) : origaddr(reinterpret_cast<FuncType>(address)) {	}

	// Initialize the object and immediately apply a hook.
	FastUserpurgeHookPtr(FuncType address, FuncType hook) : FastUserpurgeHookPtr(address)
	{
		Hook(hook);
	}

	// Initialize the object and immediately apply a hook.
	FastUserpurgeHookPtr(intptr_t address, FuncType hook) : FastUserpurgeHookPtr(address)
	{
		Hook(hook);
	}

	FastUserpurgeHookPtr() = delete;
	FastUserpurgeHookPtr(FastUserpurgeHookPtr&) = delete;
	FastUserpurgeHookPtr(FastUserpurgeHookPtr&&) = delete;

	// Apply a hook to the unhooked function.
	void Hook(FuncType hook)
	{
		if (Original)
			throw new std::exception("Cannot apply hook to already hooked function!");
		Original = (FuncType)GenerateUserpurgeWrapperC(TRetReg, (intptr_t)GenerateTrampoline((uint8_t*)origaddr), sizeof...(TRegs), TRegs...);
		GenerateUserpurgeHookC(hook, TRetReg, (intptr_t)origaddr, sizeof...(TRegs), TRegs...);
	}

	// Call the original function, bypassing the hook.
	FuncType Original = nullptr;

private:
	const FuncType origaddr;
};

/// <summary>
/// A non-standard calling convention function wrapper class.
/// </summary>
/// <typeparam name="TRet">The return type.</typeparam>
/// <typeparam name="TArgs">Type of each argument in left to right.</typeparam>
template<typename TRet, typename... TArgs>
class UsercallFunction
{
public:
	typedef void(*FuncType)(TArgs...);

	template<typename... TRegs>
	UsercallFunction(FuncType address, int ret, TRegs... regs)
	{
		Initialize((intptr_t)address, ret, regs);
	}

	template<typename... TRegs>
	UsercallFunction(intptr_t address, int ret, TRegs... regs)
	{
		Initialize(address, ret, regs);
	}

	UsercallFunction() = delete;
	UsercallFunction(UsercallFunction&) = delete;
	UsercallFunction(UsercallFunction&&) = delete;

	TRet operator()(TArgs... args)
	{
		return Wrapper(args);
	}

	FuncType operator&() \
	{
		return Wrapper;
	}

private:
	// Call the original function, bypassing the hook.
	FuncType Wrapper = nullptr;

	// Apply a hook to the unhooked function.
	template<typename... TRegs>
	void Initialize(intptr_t address, int ret, TRegs... regs)
	{
		Wrapper = (FuncType)GenerateUsercallWrapperC(ret, address, sizeof...(regs), regs...);
	}
};

/// <summary>
/// A non-standard calling convention function wrapper class.
/// </summary>
/// <typeparam name="TArgs">Type of each argument in left to right.</typeparam>
template<typename... TArgs>
class UsercallFunction<void, TArgs...>
{
public:
	typedef void(*FuncType)(TArgs...);

	template<typename... TRegs>
	UsercallFunction(FuncType address, TRegs... regs)
	{
		Initialize((intptr_t)address, regs);
	}

	template<typename... TRegs>
	UsercallFunction(intptr_t address, TRegs... regs)
	{
		Initialize(address, regs...);
	}

	UsercallFunction() = delete;
	UsercallFunction(UsercallFunction&) = delete;
	UsercallFunction(UsercallFunction&&) = delete;

	void operator()(TArgs... args)
	{
		Wrapper(args...);
	}

	FuncType operator&() \
	{
		return Wrapper;
	}

private:
	// Call the original function, bypassing the hook.
	FuncType Wrapper = nullptr;

	// Apply a hook to the unhooked function.
	template<typename... TRegs>
	void Initialize(intptr_t address, TRegs... regs)
	{
		Wrapper = (FuncType)GenerateUsercallWrapperC(noret, address, sizeof...(regs), regs...);
	}
};

/// <summary>
/// A non-standard calling convention function wrapper class with callee cleanup.
/// </summary>
/// <typeparam name="TRet">The return type.</typeparam>
/// <typeparam name="TArgs">Type of each argument in left to right.</typeparam>
template<typename TRet, typename... TArgs>
class UserpurgeFunction
{
public:
	typedef void(*FuncType)(TArgs...);

	template<typename... TRegs>
	UserpurgeFunction(FuncType address, int ret, TRegs... regs)
	{
		Initialize((intptr_t)address, ret, regs);
	}

	template<typename... TRegs>
	UserpurgeFunction(intptr_t address, int ret, TRegs... regs)
	{
		Initialize(address, ret, regs);
	}

	UserpurgeFunction() = delete;
	UserpurgeFunction(UserpurgeFunction&) = delete;
	UserpurgeFunction(UserpurgeFunction&&) = delete;

	TRet operator()(TArgs... args)
	{
		return Wrapper(args);
	}

	FuncType operator&() \
	{
		return Wrapper;
	}

private:
	// Call the original function, bypassing the hook.
	FuncType Wrapper = nullptr;

	// Apply a hook to the unhooked function.
	template<typename... TRegs>
	void Initialize(intptr_t address, int ret, TRegs... regs)
	{
		Wrapper = (FuncType)GenerateUserpurgeWrapperC(ret, address, sizeof...(regs), regs...);
	}
};

/// <summary>
/// A non-standard calling convention function wrapper class with callee cleanup.
/// </summary>
/// <typeparam name="TArgs">Type of each argument in left to right.</typeparam>
template<typename... TArgs>
class UserpurgeFunction<void, TArgs...>
{
public:
	typedef void(*FuncType)(TArgs...);

	template<typename... TRegs>
	UserpurgeFunction(FuncType address, TRegs... regs)
	{
		Initialize((intptr_t)address, regs);
	}

	template<typename... TRegs>
	UserpurgeFunction(intptr_t address, TRegs... regs)
	{
		Initialize(address, regs...);
	}

	UserpurgeFunction() = delete;
	UserpurgeFunction(UserpurgeFunction&) = delete;
	UserpurgeFunction(UserpurgeFunction&&) = delete;

	void operator()(TArgs... args)
	{
		Wrapper(args...);
	}

	FuncType operator&() \
	{
		return Wrapper;
	}

private:
	// Call the original function, bypassing the hook.
	FuncType Wrapper = nullptr;

	// Apply a hook to the unhooked function.
	template<typename... TRegs>
	void Initialize(intptr_t address, TRegs... regs)
	{
		Wrapper = (FuncType)GenerateUserpurgeWrapper<FuncType>(noret, address, regs...);
	}
};

/// <summary>
/// A thiscall function wrapper class.
/// </summary>
/// <typeparam name="TRet">The return type.</typeparam>
/// <typeparam name="TArgs">Type of each argument in left to right.</typeparam>
template<typename TRet, typename... TArgs>
class ThiscallFunction
{
public:
	typedef void(*FuncType)(TArgs...);

	template<typename... TRegs>
	ThiscallFunction(FuncType address, int ret, TRegs... regs)
	{
		Initialize((intptr_t)address, ret, regs);
	}

	template<typename... TRegs>
	ThiscallFunction(intptr_t address, int ret, TRegs... regs)
	{
		Initialize(address, ret, regs);
	}

	ThiscallFunction() = delete;
	ThiscallFunction(ThiscallFunction&) = delete;
	ThiscallFunction(ThiscallFunction&&) = delete;

	TRet operator()(TArgs... args)
	{
		return Wrapper(args);
	}

	FuncType operator&() \
	{
		return Wrapper;
	}

private:
	// Call the original function, bypassing the hook.
	FuncType Wrapper = nullptr;

	// Apply a hook to the unhooked function.
	template<typename... TRegs>
	void Initialize(intptr_t address, int ret, TRegs... regs)
	{
		Wrapper = (FuncType)GenerateThiscallWrapper(ret, sizeof...(regs) - 1);
	}
};

/// <summary>
/// A thiscall function wrapper class.
/// </summary>
/// <typeparam name="TArgs">Type of each argument in left to right.</typeparam>
template<typename... TArgs>
class ThiscallFunction<void, TArgs...>
{
public:
	typedef void(*FuncType)(TArgs...);

	template<typename... TRegs>
	ThiscallFunction(FuncType address, TRegs... regs)
	{
		Initialize((intptr_t)address, regs);
	}

	template<typename... TRegs>
	ThiscallFunction(intptr_t address, TRegs... regs)
	{
		Initialize(address, regs...);
	}

	ThiscallFunction() = delete;
	ThiscallFunction(ThiscallFunction&) = delete;
	ThiscallFunction(ThiscallFunction&&) = delete;

	void operator()(TArgs... args)
	{
		Wrapper(args...);
	}

	FuncType operator&() \
	{
		return Wrapper;
	}

private:
	// Call the original function, bypassing the hook.
	FuncType Wrapper = nullptr;

	// Apply a hook to the unhooked function.
	template<typename... TRegs>
	void Initialize(intptr_t address, TRegs... regs)
	{
		Wrapper = (FuncType)GenerateThiscallWrapper(noret, sizeof...(regs) - 1);
	}
};