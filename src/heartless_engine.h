#pragma once
#include <array>
#include <types.h>
#include <cop0.h>

//Forward declaration
class Emulator;

//Bitfield to obtain required parameters for i,j and r type encodings
//TODO: make op parameter accessible
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

	struct cop_0 //COP0 Instruction
	{
		unsigned pad0 : 11; //Not used
		unsigned rd : 5;
		unsigned rt : 5;
		unsigned cop_op : 5;
		unsigned pad1 : 6; //Not used
	} cop0;
};

//TODO: separate COP0 into its own class
class HeartlessEngine
{
public:
	Emulator* psx;
	cop0 cp0;

	std::array<u32, 32> gpr; //R0 is always 0, R31 is link reg
	u32 HI; //Stores mult high result/div remainder
	u32 LO; //Stores mult low/div quotient
	u32 pc; //Program counter
	u32 next_pc; //Simulating the pipeline

	HeartlessEngine(Emulator* psx);
	~HeartlessEngine();
	u32 get_gpr(u32 reg);
	void set_gpr(u32 reg, u32 value);
	void intepret();
	void decode_execute(Instruction instr);
	void reset();

	//INSTRUCTIONS

	//Load and store Instructions
	void SW(Instruction instr);

	//Computational Instructions
	void LUI(Instruction instr);
	void SLL(Instruction instr);
	void OR(Instruction instr);
	void ORI(Instruction instr);
	void ADDI(Instruction instr);
	void ADDIU(Instruction instr);

	//Jump and Branch Instructions
	void J(Instruction instr);
	void BNE(Instruction instr);

	//Special
};