#include <iostream>
#include <string>
#include <random>
#include <thread>
#include <atomic>
#include <vector>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/random.hpp>
#include "miller_rabin.hpp"

using boost::multiprecision::cpp_int;

bool divisible_by_small_primes(const cpp_int& n) {
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

cpp_int generate_candidate(int digits, boost::random::mt19937& rng) {
    boost::random::uniform_int_distribution<int> dist_digit(0, 9);
    boost::random::uniform_int_distribution<int> dist_first(1, 9);
    boost::random::uniform_int_distribution<int> dist_odd(0, 4);
    static const char odd_digits[5] = {'1', '3', '5', '7', '9'};

    cpp_int candidate;
    do {
        std::string s;
        s += '0' + dist_first(rng); // first digit non-zero
        for (int i = 1; i < digits - 1; ++i)
            s += '0' + dist_digit(rng);
        s += odd_digits[dist_odd(rng)]; // last digit odd
        candidate = cpp_int(s);
    } while (divisible_by_small_primes(candidate));

    return candidate;
}

int main(int argc, char** argv) {
    constexpr int rounds = 5;

    if (argc < 2 || argc > 3) {
        std::cerr << "Usage: makeprime <digits> [--twin]\n";
        return 1;
    }

    int digits = std::stoi(argv[1]);
    if (digits < 3) {
        std::cerr << "Number of digits must be at least 3.\n";
        return 1;
    }

    bool want_twin = false;
    want_twin = argc > 2 && std::string(argv[2]) == "--twin";
    int checked_mod = want_twin ? 10'000 : 20;

    std::atomic<bool> found(false);
    cpp_int result;
    std::atomic<std::size_t> total_checked = 0;
    std::mutex result_mutex;
    std::mutex output_mutex;

    auto worker = [&]() {
        boost::random::mt19937 rng(std::random_device{}());
        const cpp_int lower_limit = cpp_int("1" + std::string(digits - 1, '0'));
        const cpp_int upper_limit = cpp_int("1" + std::string(digits, '0'));

        cpp_int candidate = generate_candidate(digits, rng);
        cpp_int stride = 2 * boost::random::uniform_int_distribution<int>(1, 500)(rng); // random odd stride

        while (!found.load()) {
            if (++total_checked % checked_mod == 0) {
                std::lock_guard<std::mutex> out_lock(output_mutex);
                std::cout << '*' << std::flush;
            }

            if (!divisible_by_small_primes(candidate) &&
                fudmottin::millerRabinTest(candidate, rounds, rng) &&
                (!want_twin ||
                 (!divisible_by_small_primes(candidate + 2) &&
                  fudmottin::millerRabinTest(candidate + 2, rounds, rng)))) {

                if (!found.exchange(true)) {
                    std::lock_guard<std::mutex> lock(result_mutex);
                    result = candidate;
                    std::lock_guard<std::mutex> out_lock(output_mutex);
                    std::cout << '\n';
                }
                break;
            }

            candidate += stride;
            if (candidate >= upper_limit) {
                candidate = generate_candidate(digits, rng);
                stride = 2 * boost::random::uniform_int_distribution<int>(1, 500)(rng); // new stride
            }
        }
    };

    const unsigned num_threads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;

    std::cout << "Number of threads: " << num_threads << std::endl;
    for (unsigned i = 0; i < num_threads; ++i) {
        threads.emplace_back(worker);
    }

    for (auto& t : threads) {
        t.join();
    }

    std::cout << result << '\n';
    if (want_twin)
        std::cout << result + 2 << '\n';

    return 0;
}

