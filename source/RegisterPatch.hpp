#pragma once

// Register a function at static initializing to be run at mod initialization to respect mod order
class RegisterPatch final
{
private:
	void Register(void(*init)(), void(*free)(), void(*exec)());
public:
	RegisterPatch(void(*init)()) { Register(init, nullptr, nullptr); };
	RegisterPatch(void(*init)(), void(*free)()) { Register(init, free, nullptr); };
	RegisterPatch(void(*init)(), void(*free)(), void(*exec)()) { Register(init, free, exec); };
};

void InitPatches();
void ExecPatches();
void FreePatches();