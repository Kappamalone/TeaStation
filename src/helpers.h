#pragma once
#include <types.h>

namespace helpers
{
	//TODO: what does constexpr do

	//Reads length-variable value from a vector
	template <typename T>
	static constexpr auto read_vector(u8* mem, u32 addr) -> T
	{
		return *reinterpret_cast<T*>(&mem[addr]);
	}

	//Writes length-variable value to a vector
	template <typename T>
	static constexpr auto write_vector(u8* mem, u32 addr, T value)
	{
		*reinterpret_cast<T*>(&mem[addr]) = value;
	}


	template <typename T>
	static constexpr auto sign_extend_to_u32(T value) -> u32
	{
		return (u32)(s32)(T)(value);
	}

	//Checks if bit is set from lsb
	template <typename T>
	static constexpr auto bitset(T value, int place) -> bool
	{
		return value & (1 << place);
	}
}