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