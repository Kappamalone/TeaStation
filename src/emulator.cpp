#include <emulator.h>

Emulator::Emulator() : he_cpu(this), bus(this)
{
}

Emulator::~Emulator() {};

void Emulator::reset()
{
	he_cpu.reset();
	bus.reset();
}

void Emulator::run_Emulator()
{
	while (true)
	{
		he_cpu.intepret();
	}
}