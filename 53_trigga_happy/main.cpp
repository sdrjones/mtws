#include "ComputerCard.h"
#include "TriggaHappy.h"

int main()
{
	// In Pico SDK 2.1.1 we get a 200MHz clock through
	// PICO_USE_FASTEST_SUPPORTED_CLOCK=1
	// int he main CMakeLists.txt file.
	//set_sys_clock_khz(160000, true); // from Chris J Utility Pair
	TriggaHappy sh;
	sh.EnableNormalisationProbe();
	sh.Run();
}
