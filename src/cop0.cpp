#include <array>
#include <cop0.h>
#include <types.h>
#include <heartless_engine.h>

cop0::cop0(HeartlessEngine* he_cpu)
{
	this->he_cpu = he_cpu;
	reset();
}

cop0::~cop0()
{
}

void cop0::reset()
{
	std::fill(cp_regs.begin(), cp_regs.end(), 0);
}

const char* cop0::reg_name(u32 reg)
{
	static const char* reg_names[] =
	{
		"N/A","N/A","N/A","BPC",
		"N/A","BDA","JUMPDEST","DCIC",
		"BADV","BDAM","N/A","BPCM",
		"SR","CAUSE","EPC","PRID"
	};
	return reg_names[reg];
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

//Move to coprocessor 0
//Moves CPU reg rt into COP0 reg rd
void cop0::MTC0(Instruction instr)
{
	auto rt = instr.cop0.rt;
	auto rd = instr.cop0.rd;
	cp_regs[rd] = he_cpu->get_gpr(rt);
	printf("[COP0] $%02X Value: $%08X, COP0 Reg: %s\n", rt, he_cpu->get_gpr(rt), reg_name(rd));
}