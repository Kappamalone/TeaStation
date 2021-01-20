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
	if (reg == load_d_slot[0])
	{
		clear_load_delay(); //Direct register write overwrites load delay slot
	}

	gpr[reg] = value;
	execute_load_delay();
	clear_load_delay();
	gpr[0] = 0;
}

void HeartlessEngine::set_load_delay(u32 reg, u32 value)
{
	load_d_slot[0] = reg;
	load_d_slot[1] = value;
}

void HeartlessEngine::execute_load_delay()
{
	gpr[load_d_slot[0]] = load_d_slot[1];
}

void HeartlessEngine::clear_load_delay()
{
	load_d_slot[0] = 0;
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
		case 0b000000: SLL(instr);  break;
		case 0b100001: ADDU(instr); break;
		case 0b100101: OR(instr);   break;
		case 0b101011: SLTU(instr); break;
		default:
			printf("[CPU] Unimplemented opcode : %08X\n", instr.raw);
			exit(1);
		}
		break;
	case 0b000010: J(instr);     break;
	case 0b000011: JAL(instr);   break;
	case 0b000101: BNE(instr);   break;
	case 0b001000: ADDI(instr);  break;
	case 0b001001: ADDIU(instr); break;
	case 0b001111: LUI(instr);   break;
	case 0b001100: ANDI(instr);  break;
	case 0b001101: ORI(instr);   break;
	case 0b101000: SB(instr);    break;
	case 0b101001: SH(instr);    break;
	case 0b101011: SW(instr);    break;
	case 0b100011: LW(instr);    break;
	case 0b010000: cp0.decode_execute(instr); break;
	default:
		printf("[CPU] Unimplemented opcode : %08X\n", instr.raw);
		exit(1);
	}
}

//INSTRUCTIONS========================================
//Load and store Instructions

//IMPORTANT: Every instruction has to have a set_gpr, or the load delay slot won't be updated
//IMPORTANT: Multiple LW's won't update target register, only after the last LW
//TODO: Do something about alignments

//TODO: Template the store/loads >:)
//Stores byte
void HeartlessEngine::SB(Instruction instr)
{
	if (helpers::bitset(cp0.cp_regs[SR], 16))
	{
		printf("CACHE WRITE IGNORED\n");
		return;
	}

	auto base = instr.i.rs;
	auto target = instr.i.rt;
	auto addr = helpers::sign_extend16(instr.i.imm) + get_gpr(base);
	psx->bus.write_value<u8>(addr, get_gpr(target) & 0xff);
	set_gpr(0, 0); //To update load delay slot
	printf("%08X | SB: $%02X, $%08X\n", pc - 4, target, addr);
}

//Stores half word
void HeartlessEngine::SH(Instruction instr)
{
	if (helpers::bitset(cp0.cp_regs[SR], 16))
	{
		printf("CACHE WRITE IGNORED\n");
		return;
	}

	auto base = instr.i.rs;
	auto target = instr.i.rt;
	auto addr = helpers::sign_extend16(instr.i.imm) + get_gpr(base);
	psx->bus.write_value<u16>(addr, get_gpr(target) & 0xffff);
	set_gpr(0, 0); //To update load delay slot
	printf("%08X | SH: $%02X, $%08X\n", pc - 4, target, addr);
}

//Store Word
//Stores word at addr+s16
void HeartlessEngine::SW(Instruction instr)
{
	//SR handles whether or not the write goes to cache or memory
	if (helpers::bitset(cp0.cp_regs[SR], 16))
	{
		printf("CACHE WRITE IGNORED\n");
		return;
	}

	auto source = instr.i.rs;
	auto target = instr.i.rt;
	auto addr = helpers::sign_extend16(instr.i.imm) + get_gpr(source);
	psx->bus.write_value<u32>(addr, get_gpr(target));
	set_gpr(0, 0); //To update load delay slot
	printf("%08X | SW: $%02X, $%08X\n", pc - 4, target, addr);
}

//Load Word
//Loads words into memory at addr+s16
void HeartlessEngine::LW(Instruction instr)
{
	if (helpers::bitset(cp0.cp_regs[SR], 16))
	{
		printf("CACHE READ IGNORED\n");
		return;
	}
	auto base = instr.i.rs;
	auto target = instr.i.rt;
	auto offset = helpers::sign_extend16(instr.i.imm);
	auto word = psx->bus.read_value<u32>(offset + get_gpr(base));
	set_load_delay(target, word);
	printf("%08X | LW: $%02X, $%02X, $%08X\n", pc-4,target, base, offset);
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

void HeartlessEngine::ANDI(Instruction instr)
{
	auto imm = instr.i.imm;
	auto source = instr.i.rs;
	auto target = instr.i.rt;
	set_gpr(target, get_gpr(source) & imm);
	printf("%08X | ANDI: $%02X, $%02X, $%02X\n", pc - 4, target, source, imm);
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
	printf("%08X | ORI: $%02X, $%02X, $%02X\n", pc - 4, target, source, imm);
}

//Add unsigned
void HeartlessEngine::ADDU(Instruction instr)
{
	auto source = instr.r.rs;
	auto target = instr.r.rt;
	auto dest = instr.r.rd;
	set_gpr(dest, get_gpr(source) + get_gpr(target));
	printf("%08X | ADDU: $%02X, $%02X, $%02X", pc - 4, dest, source, target);
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

//Set on less than
void HeartlessEngine::SLTU(Instruction instr)
{
	auto source = instr.r.rs;
	auto target = instr.r.rt;
	auto dest = instr.r.rd;
	auto res = get_gpr(source) < get_gpr(target);
	set_gpr(dest, res);
	printf("%08X | SLTU: $%02X, $%02X, $%08X\n", pc - 4, dest, source, target);
}

//Jump and Branch Instructions===========================================

//Jump
//Jumps to PC top 4 bits || jump target shifted left by 2 bits
void HeartlessEngine::J(Instruction instr)
{
	auto addr = (pc & 0xf0000000) | (instr.j.target << 2);
	next_pc = addr;
	set_gpr(0, 0); //To update load delay slot
	printf("%08X | J: $%08X\n", pc - 4, addr);
}

//Jump and link
//Jump and store return addr at $31
void HeartlessEngine::JAL(Instruction instr)
{
	//TODO: write a test for this
	set_gpr(31, next_pc);
	J(instr);
	set_gpr(0, 0); //To update load delay slot
	printf("JAL\n");
}

//Branch if not equal
//Branches to relative addr if two regs aren't equal
//TODO: write a branch function
void HeartlessEngine::BNE(Instruction instr)
{
	auto taken = false;
	auto source = instr.i.rs;
	auto target = instr.i.rt;
	auto offset = helpers::sign_extend16(instr.i.imm << 2);
	if (get_gpr(source) != get_gpr(target))
	{
		//Explanation of branches for myself 
		//With the current implementation, pc is set to next_pc every instruction cycle.
		//Therefore next_pc is actually +4 ahead of where it needs to be.
		//To compensate for this, we -4 so we get the correct address to branch from
		taken = true;
		next_pc += offset - 4;
	}
	set_gpr(0, 0); //To update load delay slot
	printf("%08X | BNE [%s]: $%02X, $%02X, $%08X\n", pc - 4, taken ? "taken" : "not taken", source, target, next_pc-4);
}

//SPECIAL============================================