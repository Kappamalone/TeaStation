#include <stdio.h>
#include <doctest.h>
#include <emulator.h>

//TODO: rename `emulator` to `Teastation` for extra style points
int main()
{
	//Doctest
	/*doctest::Context ctx;
	ctx.setOption("abort-after", 5);
	ctx.applyCommandLine(__argc, __argv);
	ctx.setOption("no-breaks", true);
	int res = ctx.run();
	if (ctx.shouldExit())
		return res;*/

	Emulator npsx;
	npsx.run_Emulator();

	//return res;
}