#include <utils/pow2.h>
#include <cute/cute.h>

struct utilsut_pow2_check {
	unsigned int value;
	unsigned int expect;
};

static CUTE_PNP_SUITE(utilsut, NULL);

CUTE_PNP_TEST(utilsut_upper_pow2, &utilsut)
{
	unsigned int              c;
	struct utilsut_pow2_check checks[] = {
		{ .value = 1,              .expect = 0 },
		{ .value = (1U << 1),      .expect = 1 },
		{ .value = (1U << 1) + 1,  .expect = 1 },
		{ .value = (1U << 2),      .expect = 2 },
		{ .value = (1U << 2) + 1,  .expect = 2 },
		{ .value = (1U << 3) - 1,  .expect = 2 },
		{ .value = (1U << 3),      .expect = 3 },
		{ .value = (1U << 3) + 1,  .expect = 3 },
		{ .value = (1U << 30) - 1, .expect = 29 },
		{ .value = (1U << 30),     .expect = 30 },
		{ .value = (1U << 30) + 1, .expect = 30 },
		{ .value = (1U << 31) - 1, .expect = 30 },
		{ .value = (1U << 31),     .expect = 31 },
		{ .value = (1U << 31) + 1, .expect = 31 }
	};

	for (c = 0; c < array_nr(checks); c++)
		cute_ensure(pow2_lower(checks[c].value) == checks[c].expect);
}

CUTE_PNP_TEST(utilsut_lower_pow2, &utilsut)
{
	unsigned int              c;
	struct utilsut_pow2_check checks[] = {
		{ .value = 1,              .expect = 0 },
		{ .value = (1U << 1),      .expect = 1 },
		{ .value = (1U << 1) + 1,  .expect = 2 },
		{ .value = (1U << 2),      .expect = 2 },
		{ .value = (1U << 2) + 1,  .expect = 3 },
		{ .value = (1U << 3) - 1,  .expect = 3 },
		{ .value = (1U << 3),      .expect = 3 },
		{ .value = (1U << 3) + 1,  .expect = 4 },
		{ .value = (1U << 30) - 1, .expect = 30 },
		{ .value = (1U << 30),     .expect = 30 },
		{ .value = (1U << 30) + 1, .expect = 31 },
		{ .value = (1U << 31) - 1, .expect = 31 },
		{ .value = (1U << 31),     .expect = 31 }
	};

	for (c = 0; c < array_nr(checks); c++)
		cute_ensure(pow2_upper(checks[c].value) == checks[c].expect);
}
