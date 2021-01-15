#pragma once
#include <heartless_engine.h>
#include <types.h>
#include <helpers.h>
#include <consts.h>

//Forward declarations
class Emulator;

class Bus
{
public:
	Emulator* psx;
	std::vector<u8> kernel;
	std::vector<u8> kuseg;
	std::vector<u8> expansion1;
	std::vector<u8> scratchpad;
	std::vector<u8> mmio;
	std::vector<u8> bios;

	Bus(Emulator* psx);
	~Bus();
	template <typename T> auto read_value(u32 addr)->T;
	template <typename T> auto write_value(u32 addr, T value)->void;
	void load_bios();
	void reset();
};
