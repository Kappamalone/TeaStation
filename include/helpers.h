#pragma once
#include <types.h>

namespace helpers
{
	//TODO: what the hell does constexpr do
	//TODO: should all functions have trailing return types?

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

	//Sign extends u16
	static auto sign_extend16(u32 value) -> u32 {
		return (u32)(s32)(s16)(value);
	}
}