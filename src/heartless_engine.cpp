#include <heartless_engine.h>
#include <emulator.h>
#include <types.h>

HeartlessEngine::HeartlessEngine(Emulator* psx)
{
	this->psx = psx;
	reset();
}

HeartlessEngine::~HeartlessEngine()
{
}

void HeartlessEngine::reset()
{
	std::fill(gpr.begin(), gpr.end(), 0);
	HI = LO = 0;
	PC = 0xbfc00000;
}

u32 HeartlessEngine::get_gpr(u32 reg)
{
	return reg != 0 ? gpr[reg] : 0;
}

void HeartlessEngine::set_gpr(u32 reg, u32 value)
{
	if (reg != 0) {
		gpr[reg] = value;
	}
	gpr[0] = 0; //Just to be safe
}

void HeartlessEngine::intepret()
{
	Instruction instr;
	instr.raw = psx->bus.read_value<u32>(PC);
	PC += 4;

	switch (instr.i.op)
	{
	case 0b001111: LUI(instr); break;
	case 0b001101: ORI(instr); break;
	case 0b101011: SW(instr);  break;
	default:
		printf("[CPU] Unimplemented opcode : %08X\n", instr.raw);
		exit(1);
	}
}

//opcodes
void HeartlessEngine::LUI(Instruction instr)
{
	auto imm = instr.i.imm;
	auto target = instr.i.rt;
	set_gpr(target, imm << 16);
	printf("LUI: $%X, 0x%02X\n", target, imm);
}

void HeartlessEngine::ORI(Instruction instr)
{
	auto imm = instr.i.imm;
	auto source = instr.i.rs;
	auto target = instr.i.rt;
	set_gpr(target, get_gpr(source) | imm);
	printf("ORI: $%X, $%X, 0x%02X\n", source, target, imm);
}

void HeartlessEngine::SW(Instruction instr)
{
	auto addr = helpers::sign_extend16(instr.i.imm) + get_gpr(instr.i.rs);
	auto target = instr.i.rt;
	psx->bus.write_value(addr, target);
}

/*
void HeartlessEngine::(Instruction instr)
{
}
*/