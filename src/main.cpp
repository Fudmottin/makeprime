#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <vector>
#include <mutex>
#include <chrono>
#include <gmpxx.h>
#include "gmp_mr.hpp"

bool divisible_by_small_primes(const mpz_class& n) {
    static const std::array<int, 40> small_primes = {
        3, 5, 7, 11, 13, 17, 19, 23, 29,
        31, 37, 41, 43, 47, 53, 59, 61, 67, 71,
        73, 79, 83, 89, 97, 101, 103, 107, 109, 113,
        127, 131, 137, 139, 149, 151, 157, 163, 167, 173,
        179
    };

    for (int p : small_primes) {
        if (n % p == 0) return true;
    }
    return false;
}

mpz_class generate_candidate(int digits, gmp_randclass& rng) {
    static const char odd_digits[5] = {'1', '3', '5', '7', '9'};
    mpz_class rand_digit;
    mpz_class candidate;

    do {
        std::string s;
        rand_digit = rng.get_z_range(9); // first digit: 1–9
        s += '1' + mpz_get_ui(rand_digit.get_mpz_t());

        for (int i = 1; i < digits - 1; ++i) {
            rand_digit = rng.get_z_range(10); // middle digits: 0–9
            s += '0' + mpz_get_ui(rand_digit.get_mpz_t());
        }

        rand_digit = rng.get_z_range(5); // last digit: odd
        s += odd_digits[mpz_get_ui(rand_digit.get_mpz_t())];

        candidate = mpz_class(s);

    } while (divisible_by_small_primes(candidate));

    return candidate;
}

int main(int argc, char** argv) {
    constexpr int rounds = 15;

    if (argc < 2 || argc > 3) {
        std::cerr << "Usage: makeprime <digits> [--twin]\n";
        return 1;
    }

    int digits = std::stoi(argv[1]);
    if (digits < 3) {
        std::cerr << "Number of digits must be at least 3.\n";
        return 1;
    }

    bool want_twin = argc == 3 && std::string(argv[2]) == "--twin";
    int checked_mod = want_twin ? 10000 : 20;

    std::atomic<bool> found(false);
    mpz_class result;
    std::atomic<std::size_t> total_checked = 0;
    std::mutex result_mutex;
    std::mutex output_mutex;

    auto worker = [&]() {
        thread_local gmp_randclass rng(gmp_randinit_mt);
        rng.seed(static_cast<unsigned long>(time(nullptr)) + std::hash<std::thread::id>{}(std::this_thread::get_id()));

        thread_local mpz_class candidate;
        thread_local mpz_class twin_candidate;

        mpz_class lower_limit("1" + std::string(digits - 1, '0'));
        mpz_class upper_limit("1" + std::string(digits, '0'));

        candidate = generate_candidate(digits, rng);
        mpz_class stride = 2 * (rng.get_z_range(500) + 1); // random odd stride

        while (!found.load(std::memory_order_relaxed)) {
            if (++total_checked % checked_mod == 0) {
                std::lock_guard<std::mutex> out_lock(output_mutex);
                std::cout << '*' << std::flush;
            }

            if (!divisible_by_small_primes(candidate) &&
                mpz_probab_prime_p(candidate.get_mpz_t(), rounds) > 0) {

                if (want_twin) {
                    twin_candidate = candidate;
                    twin_candidate += 2;
                    if (!divisible_by_small_primes(twin_candidate) &&
                        mpz_probab_prime_p(twin_candidate.get_mpz_t(), rounds) > 0) {

                        if (!found.exchange(true)) {
                            std::lock_guard<std::mutex> result_lock(result_mutex);
                            result = candidate;
                            std::lock_guard<std::mutex> out_lock(output_mutex);
                            std::cout << '\n';
                        }
                        break;
                    }
                } else {
                    if (!found.exchange(true)) {
                        std::lock_guard<std::mutex> result_lock(result_mutex);
                        result = candidate;
                        std::lock_guard<std::mutex> out_lock(output_mutex);
                        std::cout << '\n';
                    }
                    break;
                }
            }

            candidate += stride;
            if (candidate >= upper_limit) {
                candidate = generate_candidate(digits, rng);
                stride = 2 * (rng.get_z_range(500) + 1);
            }
        }
    };

    const unsigned num_threads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;

    std::cout << "Number of threads: " << num_threads << '\n';

    auto start = std::chrono::steady_clock::now();

    for (unsigned i = 0; i < num_threads; ++i)
        threads.emplace_back(worker);

    for (auto& t : threads)
        t.join();

    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;

    std::cout << result.get_str() << '\n';

    if (want_twin) {
        result += 2;
        std::cout << result.get_str() << '\n';
    }

    std::cout << "Elapsed time: " << elapsed_seconds.count() << " seconds\n";

    return 0;
}

