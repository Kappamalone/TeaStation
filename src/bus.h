#pragma once
#include <map>
#include <heartless_engine.h>
#include <types.h>
#include <helpers.h>
#include <consts.h>

//Forward declarations
class Emulator;

//Why can't auto deduce type here?
constexpr std::array <u32, 8> REGION_MASK = { 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, //KUSEG: 2048 MB
											  0x7fffffff, //KSEG0: 512MB
											  0x1fffffff, //KSEG1: 512MB
											  0xffffffff, 0xffffffff };  //KSEG2: 1024MB

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

	std::map<u32, const char*> mmio_u8_regs;
	std::map<u32, const char*> mmio_u16_regs;
	std::map<u32, const char*> mmio_u32_regs;

	Bus(Emulator* psx);
	~Bus();
	void init_mmio_regs();
	template <typename T> auto read_value(u32 addr)->T;
	template <typename T> auto write_value(u32 addr, T value)->void;
	void load_bios();
	void reset();
};
