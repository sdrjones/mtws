#include "ComputerCard.h"
#include "TriggaHappy.h"

int main()
{
	set_sys_clock_khz(160000, true); // from Chris J Utility Pair
	TriggaHappy sh;
	sh.EnableNormalisationProbe();
	sh.Run();
}
