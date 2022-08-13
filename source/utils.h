#pragma once

template<typename T, intptr_t address>
class VariableHook
{
	using value_type = T;
	using pointer = value_type*;  // or also value_type*
	using reference = value_type&;  // or also value_type&
	
	struct iterator
	{
	private:
		VariableHook* m_ptr;
		int m_index;
	public:
		using iterator_category = std::random_access_iterator_tag;
		using difference_type = std::ptrdiff_t;

		reference operator*() const { return m_ptr->get(m_index); }
		pointer operator->() { return &m_ptr[m_index]; }

		iterator& operator++() { m_index++; return *this; }
		iterator operator++(int) { iterator tmp = *this; ++(*this); return tmp; }

		iterator& operator--() { m_index--; return *this; }
		iterator operator--(int) { iterator tmp = *this; --(*this); return tmp; }

		friend bool operator== (const iterator& a, const iterator& b) { return a.m_ptr == b.m_ptr && a.m_index == b.m_index; };
		friend bool operator!= (const iterator& a, const iterator& b) { return a.m_ptr != b.m_ptr || a.m_index != b.m_index; };

		constexpr iterator(VariableHook* ptr, int i) { m_ptr = ptr; m_index = i; }
	};

private:
	static constexpr int count = PLAYER_MAX;

	value_type data[count - 1];

public:
	VariableHook() = default;

	constexpr reference get(int i) const noexcept
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

	constexpr void clear() noexcept
	{
		std::fill(begin(), end(), 0);
	}

	constexpr pointer operator&() const noexcept { return &get(0); }
	constexpr operator pointer() const noexcept { return &get(0); }
	constexpr reference operator[](int i) const noexcept { return get(i); }

	constexpr iterator begin() noexcept { return iterator(this, 0); }
	constexpr iterator end() noexcept { return iterator(this, count); }
};

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
