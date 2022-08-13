#pragma once

template<typename T, intptr_t address>
class VariableHook
{
private:
	T data[PLAYER_MAX - 1];
public:
	VariableHook() = default;

	constexpr T& operator[](const int i) const noexcept
	{
		if (i == 0)
		{
			return *reinterpret_cast<T*>(address);
		}
		else
		{
			return (T&)data[i - 1];
		}
	}
};

#define MAKEVARMULTI(type, name, addr) \
	static type name##P2{}; \
	static type name##P3{}; \
	static type name##P4{}; \
	static type* const name##_m[PLAYER_MAX] = { (type*)addr, &##name##P2, &##name##P3, &##name##P4 };

void DrawSADXText(const char* text, __int16 y);
short tolevelnum(short num);
short toactnum(short num);
void SetAllPlayersInitialPosition();
void SetAllPlayersPosition(float x, float y, float z, Angle angy);
float GetDistance(NJS_VECTOR* v1, NJS_VECTOR* v2);
int GetClosestPlayerNum(NJS_POINT3* pos);
int GetClosestPlayerNum(float x, float y, float z);
int GetClosestPlayerNumRange(NJS_POINT3* pos, float range);
int GetClosestPlayerNumRange(float x, float y, float z, float range);
int IsPlayerInSphere(NJS_POINT3* p, float r);
int IsPlayerInSphere(float x, float y, float z, float r);
bool IsCameraInSphere(NJS_POINT3* p, float r);
bool IsCameraInSphere(float x, float y, float z, float r);
int IsPlayerOnDyncol(task* tp);
void njRotateX_(Angle ang);
void njRotateY_(Angle ang);
void njRotateZ_(Angle ang);
void ChangeActM(int amount);