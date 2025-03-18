#define COMPUTERCARD_NOIMPL
#include "ComputerCard.h"
#include "Random.h"
#include <cstdint>


uint32_t __not_in_flash_func(rnd12)()
{
	static uint32_t lcg_seed = 1;
	lcg_seed = 1664525 * lcg_seed + 1013904223;
	return lcg_seed >> 20;
}
uint32_t __not_in_flash_func(rnd24)()
{
	static uint32_t lcg_seed = 1;
	lcg_seed = 1664525 * lcg_seed + 1013904223;
	return lcg_seed >> 8;
}

int32_t __not_in_flash_func(rndi32)()
{
	static uint32_t lcg_seed = 1;
	lcg_seed = 1664525 * lcg_seed + 1013904223;
	return lcg_seed;
}