#include <stdio.h>
#include <doctest.h>
#include <Emulator.h>

int main()
{
	//Doctest
	doctest::Context ctx;
	ctx.setOption("abort-after", 5);
	ctx.applyCommandLine(__argc, __argv);
	ctx.setOption("no-breaks", true);
	int res = ctx.run();
	if (ctx.shouldExit())
		return res;

	Emulator psx;
	psx.run_Emulator();

	return res;
}