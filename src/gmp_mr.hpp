#pragma once
#include <gmpxx.h>

bool miller_rabin_gmp(const mpz_class& n, int rounds, gmp_randclass& rng);

