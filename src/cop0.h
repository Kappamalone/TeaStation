#pragma once
#include <array>
#include <types.h>

//Forward declarations
class HeartlessEngine;
union Instruction;

//Used to access COP0 regs using their respective names
enum cop0_mnemonic
{
	INDX,
	RAND,
	TLBL,
	BPC,
	CTXT,
	BDA,
	PIDMASK,
	DCIC,
	BADV,
	BDAM,
	TLBH,
	BPCM,
	SR,
	CAUSE,
	EPC,
	PRID
};

//TODO: bitfields for SR
class cop0
{
public:
	HeartlessEngine* he_cpu;
	std::array<u32, 16> cp_regs;

	cop0(HeartlessEngine* he_cpu);
	~cop0();
	void reset();
	const char* reg_name(u32 reg);
	void decode_execute(Instruction instr);
	void MTC0(Instruction instr);
};