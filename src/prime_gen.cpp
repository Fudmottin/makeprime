#include <iostream>
#include <array>
#include <vector>

constexpr bool is_prime(int n) {
    if (n < 2) return false;
    if (n == 2) return true;
    if (n % 2 == 0) return false;
    for (int i = 3; i * i <= n; i += 2) {
        if (n % i == 0) return false;
    }
    return true;
}

constexpr std::array<int, 2000> generate_primes() {
    std::array<int, 2000> primes{};
    int count = 0;
    int num = 2;
    
    while (count < 2000) {
        if (is_prime(num)) {
            primes[count++] = num;
        }
        ++num;
    }
    return primes;
}

int main() {
    constexpr auto small_primes = generate_primes();
    
    std::cout << "static const std::array<int, 2000> small_primes = {\n    ";
    for (size_t i = 0; i < small_primes.size(); ++i) {
        std::cout << small_primes[i];
        if (i < small_primes.size() - 1) {
            std::cout << ", ";
        }
        if ((i + 1) % 10 == 0 && i < small_primes.size() - 1) {
            std::cout << "\n    ";
        }
    }
    std::cout << "\n};\n";
    
    return 0;
}

