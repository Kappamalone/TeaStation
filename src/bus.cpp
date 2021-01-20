#include <iostream>
#include <fstream>
#include <bus.h>
#include <types.h>
#include <helpers.h>
#include <consts.h>

Bus::Bus(Emulator* psx)
{
	this->psx = psx;
	kernel.resize(KILOBYTE * 64);
	kuseg.resize(0x1EFFFF); //1.9 megs
	expansion1.resize(KILOBYTE * 64);
	scratchpad.resize(KILOBYTE);
	mmio.resize(KILOBYTE * 8);
	bios.resize(KILOBYTE * 512);
	load_bios();
}

Bus::~Bus()
{
}

template u8  Bus::read_value(u32 addr);
template u16 Bus::read_value(u32 addr);
template u32 Bus::read_value(u32 addr);
template <typename T> auto Bus::read_value(u32 addr) -> T
{
	//TODO: unaligned memory access
	addr &= REGION_MASK[addr >> 29];
	T read = 0;
	switch (addr)
	{
	case KERNEL_START ... KERNEL_END:
		//printf("[Memory] Addr: 0x%08X Unmapped memory read from KERNEL\n", addr);
		read = helpers::read_vector<T>(kernel.data(), addr - KERNEL_START);
		break;
	case KUSEG_START ... KUSEG_END:
		//printf("[Memory] Addr: 0x%08X Unmapped memory read from KUSEG\n", addr);
		read = helpers::read_vector<T>(kuseg.data(), addr - KUSEG_START);
		break;
	case EXPANSION1_START ... EXPANSION1_END:
		printf("[Memory] Addr: 0x%08X Unmapped memory read from EXPANSION 1\n", addr);
		break;
	case SCRATCHPAD_START ... SCRATCHPAD_END:
		printf("[Memory] Addr: 0x%08X Unmapped memory read from SCRATCHPAD\n", addr);
		break;
	case MMIO_START ... MMIO_END:
		printf("[Memory] Addr: 0x%08X Mapped-ish? memory read from MMIO\n", addr);
		read = helpers::read_vector<T>(mmio.data(), addr - MMIO_START);
		break;
	case BIOS_START ... BIOS_END:
		read = helpers::read_vector<T>(bios.data(), addr - BIOS_START);
		break;
	default:
		printf("[MEMORY][WARN] Addr: 0x%08X Unmapped memory read from BUS\n", addr);
	}
	return read;
}

template void Bus::write_value(u32 addr, u8  value);
template void Bus::write_value(u32 addr, u16 value);
template void Bus::write_value(u32 addr, u32 value);
template <typename T> auto Bus::write_value(u32 addr, T value)->void
{
	//TODO: unaligned memory access
	addr &= REGION_MASK[addr >> 29];
	switch (addr)
	{
	case KERNEL_START ... KERNEL_END:
		//printf("[Memory] Addr: 0x%08X Data: 0x%08X Unmapped memory write to KERNEL\n", addr, value);
		helpers::write_vector<T>(kernel.data(), addr - KERNEL_START,value);
		break;
	case KUSEG_START ... KUSEG_END:
		//printf("[Memory] Addr: 0x%08X Data: 0x%08X Unmapped memory write to KUSEG\n", addr, value);
		helpers::write_vector<T>(kuseg.data(), addr - KUSEG_START, value);
		break;
	case EXPANSION1_START ... EXPANSION1_END:
		printf("[Memory] Addr: 0x%08X Data: 0x%08X Unmapped memory write to EXPANSION 1\n", addr, value);
		break;
	case SCRATCHPAD_START ... SCRATCHPAD_END:
		printf("[Memory] Addr: 0x%08X Data: 0x%08X Unmapped memory write to SCRATCHPAD\n", addr, value);
		break;
	case MMIO_START ... MMIO_END:
		//Rewrite this by mapping the addresses to their respective names
		//and only allowing writes if said address exists in map
		switch (addr)
		{
		case 0x1f801000:
			printf("[MMIO][EXPANSION 1 BASEADDR]  Addr: 0x%08X Data: 0x%08X\n", addr, value);
			helpers::write_vector<T>(mmio.data(), addr - MMIO_START, value);
			break;
		case 0x1f801004:
			printf("[MMIO][EXPANSION 2 BASEADDR]  Addr: 0x%08X Data: 0x%08X\n", addr, value);
			helpers::write_vector<T>(mmio.data(), addr - MMIO_START, value);
			break;
		case 0x1f801008:
			printf("[MMIO][EXPANSION 1 DELAY/SIZE]  Addr: 0x%08X Data: 0x%08X\n", addr, value);
			helpers::write_vector<T>(mmio.data(), addr - MMIO_START, value);
			break;
		case 0x1f80100C:
			printf("[MMIO][EXPANSION 3 DELAY/SIZE] Addr: 0x%08X Data: 0x%08X\n", addr, value);
			helpers::write_vector<T>(mmio.data(), addr - MMIO_START, value);
			break;
		case 0x1f801010:
			printf("[MMIO][BIOS ROM] Addr: 0x%08X Data: 0x%08X\n", addr, value);
			helpers::write_vector<T>(mmio.data(), addr - MMIO_START, value);
			break;
		case 0x1f801014:
			printf("[MMIO][SPU_DELAY] Addr: 0x%08X Data: 0x%08X\n", addr, value);
			helpers::write_vector<T>(mmio.data(), addr - MMIO_START, value);
			break;
		case 0x1f801018:
			printf("[MMIO][CDROM_DELAY] Addr: 0x%08X Data: 0x%08X\n", addr, value);
			helpers::write_vector<T>(mmio.data(), addr - MMIO_START, value);
			break;
		case 0x1f80101C:
			printf("[MMIO][EXPANSION 2 DELAY/SIZE] Addr: 0x%08X Data: 0x%08X\n", addr, value);
			helpers::write_vector<T>(mmio.data(), addr - MMIO_START, value);
			break;
		case 0x1f801020:
			printf("[MMIO][COM_DELAY] Addr: 0x%08X Data: 0x%08X\n", addr, value);
			helpers::write_vector<T>(mmio.data(), addr - MMIO_START, value);
			break;
		case 0x1f801060:
			printf("[MMIO][RAM_SIZE] Addr: 0x%08X Data: 0x%08X\n", addr, value);
			helpers::write_vector<T>(mmio.data(), addr - MMIO_START, value);
			break;
		case 0x1f801d80:
			printf("[MMIO][MAIN VOLUME] Addr: 0x%08X Data: 0x%08X\n", addr, value);
			helpers::write_vector<T>(mmio.data(), addr - MMIO_START, value);
			break;
		case 0x1f801d82: //For some reason this 4 byte register is initialised through two SH's?
			if (std::is_same<T, u16>::value)
			{
				printf("[MMIO][MAIN VOLUME] Addr: 0x%08X Data: 0x%08X\n", addr, value);
				helpers::write_vector<T>(mmio.data(), addr - MMIO_START, value);
			}
			break;
		case 0x1f801d84:
			printf("[MMIO][Reverb Output Volume] Addr: 0x%08X Data: 0x%08X\n", addr, value);
			helpers::write_vector<T>(mmio.data(), addr - MMIO_START, value);
			break;
		case 0x1f801d86: //For some reason this 4 byte register is initialised through two SH's?
			if (std::is_same<T, u16>::value)
			{
				printf("[MMIO][Reverb Output Volume] Addr: 0x%08X Data: 0x%08X\n", addr, value);
				helpers::write_vector<T>(mmio.data(), addr - MMIO_START, value);
			}
			break;
		case 0x1f802041:
			if (std::is_same<T, u8>::value)
			{
				printf("[MMIO][PSX: POST] Addr: 0x%08X Data: 0x%08X\n", addr, value);
				helpers::write_vector<T>(mmio.data(), addr - MMIO_START, value);
			}
			break;
		default:
			printf("[Memory] Addr: 0x%08X Data: 0x%08X Unmapped memory write to MMIO\n", addr, value);
			exit(1);
		}
		break;
	case BIOS_START ... BIOS_END:
		helpers::write_vector<T>(bios.data(), addr - BIOS_START, value);
		break;
	default:
		printf("[MEMORY][WARN] Addr: 0x%08X Data: 0x%08X Unmapped memory write to BUS\n",addr,value);
	}
}

void Bus::reset()
{
	std::fill(kernel.begin(), kernel.end(), 0);
	std::fill(kuseg.begin(), kuseg.end(), 0);
	std::fill(expansion1.begin(), expansion1.end(), 0);
	std::fill(scratchpad.begin(), scratchpad.end(), 0);
	std::fill(mmio.begin(), mmio.end(), 0);
	std::fill(bios.begin(), bios.end(), 0);
	load_bios(); //BUG: bios is of incorrect size?
}

//Loading data into memory
void Bus::load_bios()
{
	//Read bios into memory
	//TODO: learn what this actualy does instead of copy pasting from SO you scrub
	std::ifstream file("bios/SCPH1001.BIN", std::ios::binary | std::ios::ate);
	if (file.fail())
		printf("Couldn't read bios!\n"); //TODO: change this to actually close program

	file.unsetf(std::ios::skipws);
	std::streampos file_size;

	file.seekg(0, std::ios::end);
	file_size = file.tellg();
	file.seekg(0, std::ios::beg);

	/*if (file_size != bios.size())
		printf("Bios is of incorrect size!\n");*/

	bios.insert(bios.begin(), std::istream_iterator<u8>(file), std::istream_iterator <u8>());
	file.close();

	file.seekg(0, std::ios::end);
}