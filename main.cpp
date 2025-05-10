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
    static const std::array<int, 100> small_primes = {
        2, 3, 5, 7, 11, 13, 17, 19, 23, 29,
        31, 37, 41, 43, 47, 53, 59, 61, 67, 71,
        73, 79, 83, 89, 97, 101, 103, 107, 109, 113,
        127, 131, 137, 139, 149, 151, 157, 163, 167, 173,
        179, 181, 191, 193, 197, 199, 211, 223, 227, 229,
        233, 239, 241, 251, 257, 263, 269, 271, 277, 281,
        283, 293, 307, 311, 313, 317, 331, 337, 347, 349,
        353, 359, 367, 373, 379, 383, 389, 397, 401, 409,
        419, 421, 431, 433, 439, 443, 449, 457, 461, 463,
        467, 479, 487, 491, 499, 503, 509, 521, 523, 541
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
    if (argc < 2 || argc > 3) {
        std::cerr << "Usage: makeprime <digits> [rounds]\n";
        return 1;
    }

    int digits = std::stoi(argv[1]);
    if (digits < 1) {
        std::cerr << "Number of digits must be at least 1.\n";
        return 1;
    }

    int rounds;
    if (digits <= 6)
        rounds = 5;   // small numbers, fewer rounds needed
    else if (digits <= 20)
        rounds = 8;
    else if (digits <= 50)
        rounds = 12;
    else
        rounds = 16;  // Default: ~1 in 2^80 chance of false positive for random odd composite

    if (argc == 3) {
        rounds = std::stoi(argv[2]);
        if (rounds < 1) {
            std::cerr << "Number of rounds must be at least 1.\n";
            return 1;
        }
    }

    std::atomic<bool> found(false);
    cpp_int result;
    std::mutex result_mutex;

    auto worker = [&]() {
        boost::random::mt19937 rng(std::random_device{}());
        const cpp_int lower_limit = cpp_int("1" + std::string(digits - 1, '0'));
        const cpp_int upper_limit = cpp_int("1" + std::string(digits, '0'));

        cpp_int candidate = generate_candidate(digits, rng);

        while (!found.load()) {
            if (!divisible_by_small_primes(candidate) &&
                fudmottin::millerRabinTest(candidate, rounds, rng)) {
                std::lock_guard<std::mutex> lock(result_mutex);
                if (!found.exchange(true)) {
                    result = candidate;
                }
                break;
            }

            candidate += 2;
            if (candidate >= upper_limit) {
                candidate = generate_candidate(digits, rng);
            }
        }
    };

    const unsigned num_threads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    for (unsigned i = 0; i < num_threads; ++i) {
        threads.emplace_back(worker);
    }

    for (auto& t : threads) {
        t.join();
    }

    std::cout << result << '\n';
    return 0;
}

