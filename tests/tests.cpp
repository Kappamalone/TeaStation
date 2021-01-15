#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest.h>
#include <emulator.h>

Emulator test_psx;

TEST_CASE("Test: Resetting Components")
{
	//Testing large vectors take too long?
	test_psx.reset();
	for (auto& i : test_psx.he_cpu.gpr)
		CHECK(i == 0);
}

TEST_CASE("TEST: Basic Instructions")
{
	test_psx.reset();
	test_psx.he_cpu.intepret();

	CHECK(test_psx.he_cpu.get_gpr(0x8) == 0x00130000); //LUI
	test_psx.he_cpu.intepret();
	CHECK(test_psx.he_cpu.get_gpr(0x8) == 0x0013243f); //ORI
	test_psx.he_cpu.intepret(); //Another LUI
	test_psx.he_cpu.intepret();
	CHECK(test_psx.bus.read_value<u32>(0x1f801010) == 0x8); //SW
}

TEST_CASE("Test: Bus Read/Writes")
{
	test_psx.reset();
	test_psx.bus.write_value<u32>(0xbfc00000, 0xaabbccdd);
	//Tests memory mirrors
	CHECK(test_psx.bus.read_value<u32>(0x1fc00000) == 0xaabbccdd);
	CHECK(test_psx.bus.read_value<u32>(0x9fc00000) == 0xaabbccdd);
	CHECK(test_psx.bus.read_value<u32>(0xbfc00000) == 0xaabbccdd);

	//Test that variable sized read and writes
	test_psx.bus.write_value<u16>(0x1fc00000, 0xffff);
	CHECK(test_psx.bus.read_value<u16>(0x1fc00000) == 0xffff);
	test_psx.bus.write_value<u8>(0x9fc00000, 0xee);
	CHECK(test_psx.bus.read_value<u8>(0xbfc00000) == 0xee);
}