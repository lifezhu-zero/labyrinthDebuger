#include <time.h>
#include <stdlib.h>
#include <stdio.h>

static unsigned long randSeed = 0;

#include "Rand.h"

static unsigned long ReverseBits(unsigned long _input)
{
	unsigned long ret = 0;
	static unsigned long mask[32] =
	{
		0x00000001,
		0x00000002,
		0x00000004,
		0x00000008,
		0x00000010,
		0x00000020,
		0x00000040,
		0x00000080,
		0x00000100,
		0x00000200,
		0x00000400,
		0x00000800,
		0x00001000,
		0x00002000,
		0x00004000,
		0x00008000,
		0x00010000,
		0x00020000,
		0x00040000,
		0x00080000,
		0x00100000,
		0x00200000,
		0x00400000,
		0x00800000,
		0x01000000,
		0x02000000,
		0x04000000,
		0x08000000,
		0x10000000,
		0x20000000,
		0x40000000,
		0x80000000
	};

	for (int i = 0; i < 32; i++)
	{
		unsigned long mid = _input & mask[i];
		if (i <= 15)
			mid <<= 31 - i * 2;
		else
			mid >>= 2 * i - 31;
		ret |= mid;
	}
	return ret>>16;
}

long Random(long min, long max)
{
	unsigned long ret = 0x7FFFFFFF;
	long div = max - min;

	if (randSeed == 0)
	{
		time_t tm = time(NULL);
		struct tm ctm;
		localtime_s(&ctm, &tm);
		randSeed = ctm.tm_hour;
		randSeed *= 60;
		randSeed += ctm.tm_min;
		randSeed *= 60;
		randSeed += ctm.tm_sec;

		randSeed = ReverseBits(randSeed);

		randSeed = 61199;//59563;// 63667;//Debug
		srand((unsigned int)randSeed);
		printf("seed :%u\r\n", randSeed);
	}

	ret = rand();
	ret *= div;
	ret /= RAND_MAX;
	ret += min;

	return ret;
}
