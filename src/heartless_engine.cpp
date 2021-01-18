#include <heartless_engine.h>
#include <emulator.h>
#include <types.h>

HeartlessEngine::HeartlessEngine(Emulator* psx) : cp0(this)
{
	this->psx = psx;
	reset();
	cp0.reset();
}

HeartlessEngine::~HeartlessEngine()
{
}

void HeartlessEngine::reset()
{
	cp0.reset();
	std::fill(gpr.begin(), gpr.end(), 0);
	HI = 0;
	LO = 0;
	pc = 0xbfc0'0000;
	next_pc = 0xbfc0'0004;
}

u32 HeartlessEngine::get_gpr(u32 reg)
{
	return reg != 0 ? gpr[reg] : 0;
}

void HeartlessEngine::set_gpr(u32 reg, u32 value)
{
	gpr[reg] = value;
	gpr[0] = 0; //Just to be safe
}

void HeartlessEngine::intepret()
{
	//TODO: Branch delay slot has incorrect address when logging
	Instruction instr;
	instr.raw = psx->bus.read_value<u32>(pc);
	pc = next_pc;
	next_pc += 4;

	decode_execute(instr);
}

void HeartlessEngine::decode_execute(Instruction instr)
{
	switch (instr.i.op)
	{
	case 0b000000:
		//Special - Determined by lower 5 bits
		switch (instr.r.funct)
		{
		case 0b000000: SLL(instr); break;
		case 0b100101: OR(instr);  break;
		default:
			printf("[CPU] Unimplemented opcode : %08X\n", instr.raw);
			exit(1);
		}
		break;
	case 0b000010: J(instr);     break;
	case 0b000101: BNE(instr);   break;
	case 0b001000: ADDI(instr);  break;
	case 0b001001: ADDIU(instr); break;
	case 0b001111: LUI(instr);   break;
	case 0b001101: ORI(instr);   break;
	case 0b101011: SW(instr);    break;
	case 0b010000: cp0.decode_execute(instr); break;
	default:
		printf("[CPU] Unimplemented opcode : %08X\n", instr.raw);
		exit(1);
	}
}

//INSTRUCTIONS========================================
//Load and store Instructions

//Store Word
//Stores word at addr+s16
void HeartlessEngine::SW(Instruction instr)
{
	//SR handles whether or not the write goes to cache or memory
	if (!helpers::bitset(cp0.cp_regs[SR], 16)) 
	{
		auto source = instr.i.rs;
		auto addr = helpers::sign_extend16(instr.i.imm) + get_gpr(source);
		auto target = instr.i.rt;
		psx->bus.write_value(addr, get_gpr(target));
		printf("%08X | SW: $%02X, $%08X\n", pc - 4, target, addr);
	}
	else
	{
		printf("CACHE WRITE IGNORED\n");
	}
}

//Computational Instructions=================================================

//Load upper Immediate
//Loads immediate value into upper 16 bits of rt
void HeartlessEngine::LUI(Instruction instr)
{
	auto imm = instr.i.imm;
	auto target = instr.i.rt;
	set_gpr(target, imm << 16);
	printf("%08X | LUI: $%X, $%02X\n", pc - 4, target, imm);
}

void HeartlessEngine::SLL(Instruction instr)
{
	auto source = instr.r.rs;
	auto destination = instr.r.rd;
	set_gpr(destination, get_gpr(source) << instr.r.shamt);
	printf("%08X | SLL: $%02X, $%02X, $%X\n", pc - 4, destination, source, instr.r.shamt);
}

//OR
//Bitwise logical OR
void HeartlessEngine::OR(Instruction instr)
{
	auto source = instr.r.rs;
	auto target = instr.r.rt;
	auto destination = instr.r.rd;
	set_gpr(destination, get_gpr(source) | get_gpr(target));
	printf("%08X | OR: $%02X, $%02X, $%02X\n", pc - 4, destination, source, target);
}

//OR Immediate
//Bitwise OR immediate value with rs and store in rt
void HeartlessEngine::ORI(Instruction instr)
{
	auto imm = instr.i.imm;
	auto source = instr.i.rs;
	auto target = instr.i.rt;
	set_gpr(target, get_gpr(source) | imm);
	printf("%08X | ORI: $%X, $%X, $%02X\n", pc - 4, target, source, imm);
}

//Add Immediate
//Ads imm+s16 and traps on signed overflow
void HeartlessEngine::ADDI(Instruction instr)
{
	auto source_reg = get_gpr(instr.i.rs);
	auto target = instr.i.rt;
	auto imm = helpers::sign_extend16(instr.i.imm);
	auto result = source_reg + imm;
	
	bool overflow = ((source_reg ^ result) & (imm ^ result)) >> 31; //TODO: understand this better
	if (overflow)
	{
		printf("ADDI | Raise exception!\n");
		return;
	}
	
	set_gpr(target, result);
	printf("%08X | ADDI: $%X, $%X, $%04X\n", pc - 4, target,source_reg,imm);
}

//Add Upper Immediate
//Adds imm+s16 and stores to rt
void HeartlessEngine::ADDIU(Instruction instr)
{
	auto source = instr.i.rs;
	auto target = instr.i.rt;
	auto imm = instr.i.imm;
	set_gpr(target, helpers::sign_extend16(imm) + get_gpr(source));
	printf("%08X | ADDIU: $%02X, $%02X, $%08X\n", pc - 4, target, source, imm);
}

//Jump and Branch Instructions===========================================

//Jump
//Jumps to PC top 4 bits || jump target shifted left by 2 bits
void HeartlessEngine::J(Instruction instr)
{
	auto addr = (pc & 0xf0000000) | (instr.j.target << 2);
	next_pc = addr;
	printf("%08X | J: $%08X\n", pc - 4, addr);
}

//Branch if not equal
//Branches to relative addr if two regs aren't equal
//TODO: right branch function
void HeartlessEngine::BNE(Instruction instr)
{
	auto taken = false;
	auto source = instr.i.rs;
	auto target = instr.i.rt;
	auto offset = helpers::sign_extend16(instr.i.imm << 2);
	if (get_gpr(source) != get_gpr(target))
	{
		taken = true;
		next_pc += offset - 4; //-4 to compensate for the auto increment done by fetching instructions
	}
	printf("%08X | BNE [%s]: $%02X, $%02X, $%08X\n", pc - 4, taken ? "taken" : "not taken", source, target, offset);
}

//SPECIAL============================================