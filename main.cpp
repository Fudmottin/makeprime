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

cpp_int generate_candidate(int digits, boost::random::mt19937& rng) {
    boost::random::uniform_int_distribution<int> dist(0, 9);
    std::string s;
    s += '1' + rng() % 9;
    for (int i = 1; i < digits - 1; ++i) s += '0' + rng() % 10;
    s += "13579"[rng() % 5];
    return cpp_int(s);
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: makeprime <digits>\n";
        return 1;
    }

    int digits = std::stoi(argv[1]);
    if (digits < 1) {
        std::cerr << "Number of digits must be at least 1.\n";
        return 1;
    }

    std::atomic<bool> found(false);
    cpp_int result;
    std::mutex result_mutex;

    auto worker = [&]() {
        boost::random::mt19937 rng(std::random_device{}());
        while (!found.load()) {
            cpp_int candidate = generate_candidate(digits, rng);
            if (fudmottin::millerRabinTest(candidate)) {
                std::lock_guard<std::mutex> lock(result_mutex);
                if (!found.exchange(true)) {
                    result = candidate;
                }
                break;
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

