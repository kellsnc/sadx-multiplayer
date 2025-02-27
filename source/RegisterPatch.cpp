#include "pch.h"
#include <vector>
#include <chrono>
#include "RegisterPatch.hpp"

static std::vector<void(*)()>& getInitList() {
	static std::vector<void(*)()> g_InitList;
	return g_InitList;
}

static std::vector<void(*)()>& getExecList() {
	static std::vector<void(*)()> g_ExecList;
	return g_ExecList;
}

static std::vector<void(*)()>& getFreeList() {
	static std::vector<void(*)()> g_FreeList;
	return g_FreeList;
}

void RegisterPatch::Register(void(*init)(), void(*free)(), void(*exec)())
{
	getInitList().push_back(init);

	if (free)
	{
		getFreeList().push_back(free);
	}

	if (exec)
	{
		getExecList().push_back(exec);
	}
}

void InitPatches()
{
	PrintDebug("[MULTI] Patching many game functions...\n");

	auto start = std::chrono::high_resolution_clock::now();
	for (auto& init : getInitList())
	{
		init();
	}
	auto end = std::chrono::high_resolution_clock::now();

	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
	PrintDebug("[MULTI] Done patching in %f seconds\n", (double)duration / 1000000.0);
}

void ExecPatches()
{
	for (auto& exec : getExecList())
	{
		exec();
	}
}

void FreePatches()
{
	for (auto& free : getFreeList())
	{
		free();
	}
}