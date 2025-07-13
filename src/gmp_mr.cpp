#include "gmp_mr.hpp"

bool miller_rabin_gmp(const mpz_class& n, int rounds, gmp_randclass& rng) {
    if (n <= 3) return n == 2 || n == 3;
    if ((n & 1) == 0) return false;

    mpz_class d = n - 1;
    int r = 0;
    while ((d & 1) == 0) {
        d >>= 1;
        ++r;
    }

    for (int i = 0; i < rounds; ++i) {
        mpz_class a = rng.get_z_range(n - 3) + 2;
        mpz_class x;
        mpz_powm(x.get_mpz_t(), a.get_mpz_t(), d.get_mpz_t(), n.get_mpz_t());

        if (x == 1 || x == n - 1)
            continue;

        bool composite = true;
        for (int j = 1; j < r; ++j) {
            mpz_powm_ui(x.get_mpz_t(), x.get_mpz_t(), 2, n.get_mpz_t());
            if (x == n - 1) {
                composite = false;
                break;
            }
            if (x == 1)
                return false;
        }

        if (composite)
            return false;
    }

    return true;
}

