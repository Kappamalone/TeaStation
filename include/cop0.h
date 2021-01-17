#pragma once

//Forward declarations
class HeartlessEngine;
union Instruction;

class cop0
{
public:
	HeartlessEngine* he_cpu;

	cop0();
	~cop0();
	void decode_execute(Instruction instr);
	void MTC0(Instruction instr);
};