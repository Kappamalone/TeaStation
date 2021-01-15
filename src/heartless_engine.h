#pragma once
#include <array>
#include <types.h>

//Forward declaration
class Emulator;

//Bitfield to obtain required parameters for i,j and r type encodings
union Instruction
{
	u32 raw;
	struct i_type //Immediate
	{
		unsigned imm : 16;
		unsigned rt : 5;
		unsigned rs : 5;
		unsigned op : 6;
	} i;

	struct j_type //Jump
	{
		unsigned target : 26;
		unsigned op : 6;
	} j;

	struct r_type //Register
	{
		unsigned funct : 6;
		unsigned shamt : 5;
		unsigned rd : 5;
		unsigned rt : 5;
		unsigned rs : 5;
		unsigned op : 6;
	} r;
};

class HeartlessEngine
{
public:
	Emulator* psx;

	std::array<u32, 32> gpr; //R0 is always 0, R31 is link reg
	u32 HI; //Stores mult high result/div remainder
	u32 LO; //Stores mult low/div quotient
	u32 PC; //Program counter

	HeartlessEngine(Emulator* psx);
	~HeartlessEngine();
	u32 get_gpr(u32 reg);
	void set_gpr(u32 reg, u32 value);
	void intepret();
	void reset();

	//opcodes
	void LUI(Instruction instr);
	void ORI(Instruction instr);
	void SW(Instruction instr);
};