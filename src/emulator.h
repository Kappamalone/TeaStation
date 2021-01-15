#pragma once
#include <stdio.h>
#include <heartless_engine.h>
#include <bus.h>
#include <types.h>
#include <helpers.h>

class Emulator
{
public:
	HeartlessEngine he_cpu;
	Bus bus;

	Emulator();
	~Emulator();
	void reset();
	void run_Emulator();
};