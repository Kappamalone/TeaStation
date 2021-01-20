#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest.h>
#include <array>
#include <emulator.h>

//TODO: Does static help with psx being a global var?
static Emulator psx;

TEST_CASE("TEST: Resetting Components")
{
	//Testing large vectors take too long?
	psx.reset();
	for (auto& i : psx.he_cpu.gpr)
		CHECK(i == 0);
	for (auto& i : psx.he_cpu.cp0.cp_regs)
		CHECK(i == 0);
}

TEST_CASE("TEST: Helper Functions")
{
	u32 val = 0x10000;
	CHECK(helpers::bitset(val, 16) == true);
	CHECK(helpers::bitset(val, 15) == false);
}

TEST_CASE("TEST: Bus Read/Writes")
{
	psx.reset();
	psx.bus.write_value<u32>(0xbfc00000, 0xaabbccdd);
	//Tests memory mirrors
	CHECK(psx.bus.read_value<u32>(0x1fc00000) == 0xaabbccdd);
	CHECK(psx.bus.read_value<u32>(0x9fc00000) == 0xaabbccdd);
	CHECK(psx.bus.read_value<u32>(0xbfc00000) == 0xaabbccdd);

	//Test that variable sized read and writes
	psx.bus.write_value<u16>(0x1fc00000, 0xffff);
	CHECK(psx.bus.read_value<u16>(0x1fc00000) == 0xffff);
	psx.bus.write_value<u8>(0x9fc00000, 0xee);
	CHECK(psx.bus.read_value<u8>(0xbfc00000) == 0xee);
}

TEST_CASE("TEST: Basic Instructions")
{
	psx.reset();

	psx.he_cpu.intepret();
	CHECK(psx.he_cpu.get_gpr(0x8) == 0x00130000); //LUI
	psx.he_cpu.intepret();
	CHECK(psx.he_cpu.get_gpr(0x8) == 0x0013243f); //ORI
	psx.he_cpu.intepret();
	psx.he_cpu.intepret();
	CHECK(psx.bus.read_value<u32>(0x1f801010) == 0x0013243f); //SW
	psx.he_cpu.intepret(); //SLL, except it's a NOP
	psx.he_cpu.intepret();
	CHECK(psx.he_cpu.get_gpr(0x8) == 0x00000B88); //ADDIu
	psx.he_cpu.intepret(); //LUI
}

TEST_CASE("TEST: COP0")
{
	//Using enums to read/write specific values to cp0 regs
	psx.reset();
	psx.he_cpu.cp0.cp_regs[0] = 0xaa;
	psx.he_cpu.cp0.cp_regs[TLBL] = 0xbb;
	CHECK(psx.he_cpu.cp0.cp_regs[0] == 0xaa);
	CHECK(psx.he_cpu.cp0.cp_regs[2] == 0xbb);
}

TEST_CASE("TEST: Branch Delay Slots")
{
	//Tests that the instruction after a jump is executed
	//This will break if/when I make bios read only
	psx.reset();
	psx.bus.write_value<u32>(0xbfc0'0000, 0x08000000); //Jump to 0xB000'0000
	psx.bus.write_value<u32>(0xbfc0'0004, 0x3C01FFFF); //LUI $1 0xffff
	psx.he_cpu.intepret(); //Jump encountered
	psx.he_cpu.intepret(); //Branch delay should execute LUI
	CHECK(psx.he_cpu.get_gpr(1) == 0xffff'0000);
}

TEST_CASE("TEST: Load Delay Slots")
{
	psx.reset();
	psx.bus.write_value<u32>(0xbfc0'0010, 2);
	psx.he_cpu.gpr[10] = 0xbfc0'0010; //T2 = address of memory holding variable 2
	psx.bus.write_value<u32>(0xbfc0'0000, 0x24010001); //ADDIU $1, $0, 0x1
	psx.bus.write_value<u32>(0xbfc0'0004, 0x8D410000); //LW $1, 0($10)
	psx.bus.write_value<u32>(0xbfc0'0008, 0x24220000); //ADDIU $2, $1, 0x0
	psx.bus.write_value<u32>(0xbfc0'000C, 0x24230000); //ADDIU $3, $1, 0x0
	psx.he_cpu.intepret();
	psx.he_cpu.intepret();
	psx.he_cpu.intepret();
	psx.he_cpu.intepret();

	CHECK(psx.he_cpu.get_gpr(1) == 0x2);
	CHECK(psx.he_cpu.get_gpr(2) == 0x1); //Load delay prevents 2 from being loaded
	CHECK(psx.he_cpu.get_gpr(3) == 0x2); //Load delay finishes after 1 cycle

	psx.reset();
	psx.bus.write_value<u32>(0xbfc0'0010, 1);
	psx.he_cpu.gpr[10] = 0xbfc0'0010; //T2 = address of memory holding variable 2
	psx.bus.write_value<u32>(0xbfc0'0000, 0x8D410000); //LW $1, 0($10)
	psx.bus.write_value<u32>(0xbfc0'0004, 0x24010002); //ADDIU $1, $0, 0x2
	psx.he_cpu.intepret();
	psx.he_cpu.intepret(); //ADDIU targets load delay slot and overwrites it

	CHECK(psx.he_cpu.get_gpr(1) == 0x2);
}

TEST_CASE("TEST: Branch Instructions")
{
	//TODO
}

TEST_CASE("TEST: Random Things")
{
	auto a = (u32)(0 > 1);
	CHECK(a == 0);
	auto b = (u32)(0 < 1);
	CHECK(b == 1);
}