#include <types.h>
#include <heartless_engine.h>
#include <cop0.h>

cop0::cop0()
{
	//Do cop0 stuff
}

cop0::~cop0()
{
}

void cop0::decode_execute(Instruction instr)
{
	switch (instr.cop0.cop_op)
	{
	case 0b00100: MTC0(instr); break;
	default:
		printf("[COP0] Unimplemented opcode : %08X\n", instr.raw);
		exit(1);
	}
}

void cop0::MTC0(Instruction instr)
{
}