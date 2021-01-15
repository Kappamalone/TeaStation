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
	template <typename T> auto read_value(u32 addr)->T
	{
		addr &= REGION_MASK[addr >> 29];
		switch (addr)
		{
		case KERNEL_START ... KERNEL_END:
			printf("[Memory] Addr: 0x%08X Unmapped memory read from KERNEL\n", addr);
			break;
		case EXPANSION1_START ... EXPANSION1_END:
			printf("[Memory] Addr: 0x%08X Unmapped memory read from EXPANSION 1\n", addr);
			break;
		case SCRATCHPAD_START ... SCRATCHPAD_END:
			printf("[Memory] Addr: 0x%08X Unmapped memory read from SCRATCHPAD\n", addr);
			break;
		case MMIO_START ... MMIO_END:
			printf("[Memory] Addr: 0x%08X Unmapped memory read from MMIO\n", addr);
			break;
		case BIOS_START ... BIOS_END:
			return helpers::read_vector<T>(bios.data(), addr - BIOS_START);
			break;
		}
		return 0;
	} //TODO: explicit templat instantiation in a .cpp file
	template <typename T> auto write_value(u32 addr, T value)
	{
		addr &= REGION_MASK[addr >> 29];
		switch (addr)
		{
		case KERNEL_START ... KERNEL_END:
			printf("[Memory] Addr: 0x%08X Unmapped memory write from KERNEL\n", addr);
			break;
		case EXPANSION1_START ... EXPANSION1_END:
			printf("[Memory] Addr: 0x%08X Unmapped memory write from EXPANSION 1\n", addr);
			break;
		case SCRATCHPAD_START ... SCRATCHPAD_END:
			printf("[Memory] Addr: 0x%08X Unmapped memory write from SCRATCHPAD\n", addr);
			break;
		case MMIO_START ... MMIO_END:
			printf("[Memory] Addr: 0x%08X Unmapped memory write from MMIO\n", addr);
			break;
		case BIOS_START ... BIOS_END:
			return helpers::write_vector<T>(bios.data(), addr - BIOS_START, value);
			break;
		}
	}
	void load_bios();
	void reset();
};
