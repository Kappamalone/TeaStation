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

TEST_CASE("TEST: Branch Instructions")
{
	//TODO
}