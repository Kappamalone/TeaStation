#pragma once
#include <types.h>

namespace helpers
{
	//TODO: what the hell does constexpr do

	//Reads length-variable value from a vector
	template <typename T>
	static constexpr auto read_vector(u8* mem, u32 addr) -> T
	{
		return *reinterpret_cast<T*>(&mem[addr]);
	}

	//Writes length-variable value to a vector
	//TODO: what happens when an u32 value is given to a <u16> write?
	template <typename T>
	static constexpr auto write_vector(u8* mem, u32 addr, T value)
	{
		*reinterpret_cast<T*>(&mem[addr]) = value;
	}

	//Sign extends u16
	static auto sign_extend16(u32 value) -> u32 {
		return (u32)(s32)(s16)(value);
	}

	//Checks if bit is set from lsb
	template <typename T>
	static constexpr auto bitset(T value, int place) -> bool
	{
		return value & (1 << place);
	}
}