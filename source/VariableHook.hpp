#pragma once

#include <cstdint>
#include <algorithm>

/*

Class that expends a variable to an array of 4 components
It does so by returning a reference to the real variable is item 0 is requested,
or a reference to newly allocated data for other items

Usage:
VariableHook<type, address> name;
name[x].something = thing;

*/

template<typename T, intptr_t address>
class VariableHook final
{
	using value_type = T;
	using pointer = value_type*;
	using reference = value_type&;

	struct iterator
	{
	private:
		VariableHook& m_ptr;
		int m_index;
	public:
		using iterator_category = std::random_access_iterator_tag;
		using difference_type = std::ptrdiff_t;

		constexpr reference operator*() const { return m_ptr.get(m_index); }
		constexpr pointer operator->() { return &m_ptr.get(m_index); }

		constexpr iterator& operator++() { m_index++; return *this; }
		constexpr iterator& operator++(int) { iterator tmp = *this; ++(*this); return tmp; }

		constexpr iterator& operator--() { m_index--; return *this; }
		constexpr iterator& operator--(int) { iterator tmp = *this; --(*this); return tmp; }

		friend constexpr bool operator== (const iterator& a, const iterator& b) { return a.m_index == b.m_index; };
		friend constexpr bool operator!= (const iterator& a, const iterator& b) { return a.m_index != b.m_index; };

		constexpr iterator(VariableHook& ptr, int i) : m_ptr(ptr), m_index(i) {};
	};

private:
	static constexpr int count = 4;

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

	constexpr iterator begin() noexcept { return iterator(*this, 0); }
	constexpr iterator end() noexcept { return iterator(*this, count); }
};
