#include <iostream>
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
	clear_load_delay();
	HI = 0;
	LO = 0;
	pc = 0xbfc00000;
	next_pc = 0xbfc00004;
}

void HeartlessEngine::print_gpr()
{
	printf("\n");
	for (int i = 0; i < 16; i++)
	{
		printf("r%02d: %08X\t\tr%02d: %08X\n", i, gpr[i], i + 16, gpr[i + 16]);
	}
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
	if ((pc & 0x3) != 0)
	{
		printf("[ERROR] Unaligned PC: %08X", pc);
		exit(1);
	}


	auto breakpoint = 0;
	if (pc == breakpoint)
	{
		print_gpr();
		getchar();
		step = true;
	}
	if (!step)
	{
		Instruction instr;
		instr.raw = psx->bus.read_value<u32>(pc);
		pc = next_pc;
		next_pc += 4;

		decode_execute(instr);
	}
	else
	{
		Instruction instr;
		instr.raw = psx->bus.read_value<u32>(pc);
		pc = next_pc;
		next_pc += 4;

		decode_execute(instr);
		print_gpr();
		getchar();
	}
}

void HeartlessEngine::decode_execute(Instruction instr)
{
	switch (instr.i.op)
	{
	case 0b000000:
		//Special - Determined by lower 5 bits
		switch (instr.r.funct)
		{
		case 0b000000: SLL(instr);    break;
		case 0b001000: JR(instr);     break;
		case 0b100000: ADD(instr);    break;
		case 0b100001: ADDU(instr);   break;
		case 0b100100: AND(instr);    break;
		case 0b100101: OR(instr);     break;
		case 0b101011: SLTU(instr);   break;
		default:
			printf("[CPU] Unimplemented opcode : %08X\n", instr.raw);
			print_gpr();
			exit(1);
		}
		break;
	case 0b000010: J(instr);          break;
	case 0b000011: JAL(instr);        break;
	case 0b000100: BEQ(instr);        break;
	case 0b000101: BNE(instr);        break;
	case 0b001000: ADDI(instr);       break;
	case 0b001001: ADDIU(instr);      break;
	case 0b001111: LUI(instr);        break;
	case 0b001100: ANDI(instr);       break;
	case 0b001101: ORI(instr);        break;
	case 0b100000: LD<u8>(instr);     break;
	case 0b100011: LD<u32>(instr);    break;
	case 0b101000: SD<u8>(instr);     break;
	case 0b101001: SD<u16>(instr);    break;
	case 0b101011: SD<u32>(instr);    break;
	case 0b010000: cp0.decode_execute(instr); break;
	default:
		printf("[CPU] Unimplemented opcode : %08X\n", instr.raw);
		print_gpr();
		exit(1);
	}
}

//INSTRUCTIONS========================================
//Load and store Instructions

//IMPORTANT: Every instruction has to have a set_gpr, or the load delay slot won't be updated
//IMPORTANT: Multiple LW's won't update target register, only after the last LW
//TODO: Do something about alignments

//Store data at s16+base
template <typename T>
void HeartlessEngine::SD(Instruction instr)
{
	if (helpers::bitset(cp0.cp_regs[SR], 16))
	{
		printf("CACHE WRITE IGNORED\n");
		return;
	}

	auto base = instr.i.rs;
	auto target = instr.i.rt;
	auto addr = helpers::sign_extend_to_u32<s16>(instr.i.imm) + get_gpr(base);

	psx->bus.write_value<T>(addr, get_gpr(target));
	set_gpr(0, 0); //To update load delay slot
	printf("%08X | Store(%d): $%02X, $%08X\n", pc - 4,sizeof(T), target, addr);

	if (addr == 0xA000B9B0)
	{
		printf("found ya\n");
		printf("%08X", get_gpr(target));
		getchar();
	}
}

//Load data from s16 + base
template <typename T>
void HeartlessEngine::LD(Instruction instr)
{
	if (helpers::bitset(cp0.cp_regs[SR], 16))
	{
		printf("CACHE READ IGNORED\n");
		return;
	}

	auto base = instr.i.rs;
	auto target = instr.i.rt;
	auto addr = helpers::sign_extend_to_u32<s16>(instr.i.imm) + get_gpr(base);
	auto word = psx->bus.read_value<T>(addr);
	if (std::is_same<T, u8>::value)

		word = helpers::sign_extend_to_u32<s8>(word);
	if (std::is_same<T, u16>::value)
		word = helpers::sign_extend_to_u32<s16>(word);

	printf("value of word is: %08X addr: %08X\n", word,addr);
	set_load_delay(target, word);
	printf("%08X | Load(%d): $%02X, $%02X, $%X\n", pc - 4, sizeof(T), target, base, addr);
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
	auto source = instr.r.rt;
	auto destination = instr.r.rd;
	set_gpr(destination, get_gpr(source) << instr.r.shamt);
	printf("%08X | SLL: $%02X, $%02X, $%X\n", pc - 4, destination, source, instr.r.shamt);
}

void HeartlessEngine::AND(Instruction instr)
{
	auto source = instr.r.rs;
	auto target = instr.r.rt;
	auto destination = instr.r.rd;
	set_gpr(destination, get_gpr(source) & get_gpr(target));
	printf("%08X | AND: $%02X, $%02X, $%02X\n", pc - 4, destination, source, target);
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

void HeartlessEngine::ADD(Instruction instr)
{
	auto source_reg = get_gpr(instr.r.rs);
	auto target_reg = get_gpr(instr.r.rt);
	auto dest = instr.r.rd;
	auto res = source_reg + target_reg;
	bool overflow = ((source_reg>>31 == target_reg)>>31 && (source_reg>>31 != res>>31));
	if (overflow)
	{
		printf("ADD | Raise exception!\n");
		exit(1);
		return;
	}
	set_gpr(dest, res);
	printf("%08X | ADD: $%02X, $%02X, $%02X\n", pc - 4, dest, instr.r.rs, instr.r.rt);
}

//Add unsigned
void HeartlessEngine::ADDU(Instruction instr)
{
	auto source = instr.r.rs;
	auto target = instr.r.rt;
	auto dest = instr.r.rd;
	set_gpr(dest, get_gpr(source) + get_gpr(target));
	printf("%08X | ADDU: $%02X, $%02X, $%02X\n", pc - 4, dest, source, target);
}

//Add Immediate
//Ads imm+s16 and traps on signed overflow
void HeartlessEngine::ADDI(Instruction instr)
{
	auto source_reg = get_gpr(instr.i.rs);
	auto target = instr.i.rt;
	auto imm = helpers::sign_extend_to_u32<s16>(instr.i.imm);
	auto res = source_reg + imm;

	bool overflow = ((source_reg>>31 == imm>>31) && (source_reg>>31 != res>>31)); //TODO: understand this better
	if (overflow)
	{
		printf("ADDI | Raise exception!\n");
		exit(1);
		return;
	}

	set_gpr(target, res);
	printf("%08X | ADDI: $%X, $%X, $%04X\n", pc - 4, target, source_reg, imm);
}

//Add Upper Immediate
//Adds reg+s16 and stores to rt
void HeartlessEngine::ADDIU(Instruction instr)
{
	auto source = instr.i.rs;
	auto target = instr.i.rt;
	auto imm = instr.i.imm;
	set_gpr(target, helpers::sign_extend_to_u32<s16>(imm) + get_gpr(source));
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

void HeartlessEngine::Branch(Instruction instr)
{
	auto offset = helpers::sign_extend_to_u32<s16>(instr.i.imm << 2);
	//Explanation of branches for myself
	//With the current implementation, pc is set to next_pc every instruction cycle.
	//Therefore next_pc is actually +4 ahead of where it needs to be
	//(Which is a pointer to the next instruction) To compensate for this, we -4
	//so we get the correct address to branch from
	next_pc += offset - 4;
}

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
	printf("JAL ->");
	J(instr);
}

//Jump register
void HeartlessEngine::JR(Instruction instr)
{
	auto source = instr.r.rs;
	next_pc = get_gpr(source);
	set_gpr(0, 0); //To update load delay slot
	printf("%08X | JR: $%02X\n", pc - 4, instr.r.rs);
}

//Branch if equal

void HeartlessEngine::BEQ(Instruction instr)
{
	auto taken = false;
	auto source = instr.i.rs;
	auto target = instr.i.rt;
	if (get_gpr(source) == get_gpr(target))
	{
		taken = true;
		Branch(instr);
	}
	set_gpr(0, 0); //To update load delay slot
	printf("%08X | BEQ [%s]: $%02X, $%02X, $%08X\n", pc - 4, taken ? "taken" : "not taken", source, target, next_pc - 4);
}

//Branch if not equal
//Branches to relative addr if two regs aren't equal
void HeartlessEngine::BNE(Instruction instr)
{
	auto taken = false;
	auto source = instr.i.rs;
	auto target = instr.i.rt;
	if (get_gpr(source) != get_gpr(target))
	{
		//Explanation of branches for myself
		//With the current implementation, pc is set to next_pc every instruction cycle.
		//Therefore next_pc is actually +4 ahead of where it needs to be
		//(Which is a pointer to the next instruction) To compensate for this, we -4
		//so we get the correct address to branch from
		taken = true;
		Branch(instr);
	}
	set_gpr(0, 0); //To update load delay slot
	printf("%08X | BNE [%s]: $%02X, $%02X, $%08X\n", pc - 4, taken ? "taken" : "not taken", source, target, next_pc - 4);
}

//SPECIAL============================================