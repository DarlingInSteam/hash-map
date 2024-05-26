#include <iostream>
#include "DoubleHashingHashTable.h"
#include "LinearProbingHashTable.h"
#include "BucketHashTable.h"
#include <concepts>
#include <cassert>
#include <unordered_set>
#include <vector>
#include <ranges>
#include <chrono>

template<std::integral T>
consteval std::size_t bin_digits_amount(T number) {
    assert(number >= 0);

    std::size_t amount = 0;
    std::size_t temp = number;

    do {
        temp /= 2;
        amount += 1;
    } while (temp > 0);

    return amount;
}

bool gen_random_bool() {
    return rand() % 2 == 0;
}

std::size_t gen_random() {
    std::size_t result = 0;

    for (std::size_t i = 0; i < sizeof(std::size_t) * 8 / bin_digits_amount(RAND_MAX) + 1; ++i) {
        result |= rand() << (bin_digits_amount(RAND_MAX) * i);
    }

    return result;
}

template<std::size_t size>
std::unordered_set<std::size_t> generate_rand_numbers(std::size_t min, std::size_t max) {
    std::unordered_set<std::size_t> unique_numbers;
    unique_numbers.reserve(size);

    while (unique_numbers.size() < size) {
        unique_numbers.insert(gen_random() % (max - min) + min);
    }

    return unique_numbers;
}

template<typename K>
struct StdHasher {
    std::size_t operator()(const K &key) {
        return std::hash<K>{}(key);
    }
};

template<>
struct StdHasher<std::pair<std::size_t, bool>> {
    std::size_t operator()(const std::pair<std::size_t, bool> &key) {
        return std::hash<std::size_t>{}(key.first) ^ (std::hash<bool>{}(key.second) << 1);
    }
};

std::ostream &bold_on(std::ostream &os) {
    return os << "\e[1m";
}

std::ostream &bold_off(std::ostream &os) {
    return os << "\e[0m";
}

template<template<typename, typename, template<typename> typename, double> typename H>
void test_deletion(std::string_view title, const std::unordered_set<std::size_t> &random_numbers) {
    H<std::size_t, std::size_t, StdHasher, 0.5> hash_table;

    std::cout << bold_on << "test: " << bold_off << title << "\n";

    for (auto number: random_numbers) {
        std::cout << bold_on << "insert: " << bold_off << number << "\n";
        hash_table.insert({number, number});
    }

    std::cout << bold_on << "\nload factor: " << bold_off << hash_table.load_factor() << "\n";
    std::cout << bold_on << "hash table fullness: " << bold_off << hash_table.fullness() << "\n\n";

    for (auto number: random_numbers) {
        if (gen_random_bool()) {
            std::cout << bold_on << "try to remove: " << bold_off << number << " ... ";
            auto remove_result = hash_table.remove(number);
            std::cout << (remove_result ? "successful" : "failed") << "\n";
        }
    }

    std::cout << bold_on << "\nafter removes:\n" << bold_off;
    hash_table.debug();
}

template<template<typename, typename, template<typename> typename, double> typename H, std::size_t min, double load_factor_limit, bool count_average_probes>
void test(std::string_view title, const std::unordered_set<std::size_t> &random_numbers) {
    H<std::size_t, std::size_t, StdHasher, load_factor_limit> hash_table;

    std::size_t successful_probes_count = 0;
    std::size_t failed_probes_count = 0;

    std::size_t successful_count = 0;
    std::size_t failed_count = 0;

    std::chrono::duration<double, std::milli> successful_duration(0);
    std::chrono::duration<double, std::milli> failed_duration(0);

    for (auto number: random_numbers) {
        if (hash_table.fullness() >= min && hash_table.load_factor() >= load_factor_limit) {
            break;
        }

        hash_table.insert({number, number});
    }

    for (std::size_t i = 0; i < 1'000'000; ++i) {
        std::size_t probes_count = 0;

        auto t1 = std::chrono::high_resolution_clock::now();
        auto found_value = hash_table.find(i, probes_count);
        auto t2 = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double, std::milli> delta_t = t2 - t1;

        if (found_value) {
            successful_duration += delta_t;
            successful_probes_count += probes_count;
            ++successful_count;
        } else {
            failed_duration += delta_t;
            failed_probes_count += probes_count;
            ++failed_count;
        }
    }

    double average_successful_probes =
            static_cast<double>(successful_probes_count) / static_cast<double>(hash_table.fullness());
    double average_failed_probes =
            static_cast<double>(failed_probes_count) / static_cast<double>(1'000'000 - hash_table.fullness());

    std::cout << bold_on << "test: " << bold_off << title << "\n";

    std::cout << bold_on << "load factor limit: " << bold_off << load_factor_limit << "\n";
    std::cout << bold_on << "load factor: " << bold_off << hash_table.load_factor() << "\n";
    std::cout << bold_on << "hash table fullness: " << bold_off << hash_table.fullness() << "\n\n";

    if constexpr (count_average_probes) {
        std::cout << bold_on << "total probes count of successful searches: " << bold_off << successful_probes_count
                  << "\n";
        std::cout << bold_on << "average probes count of successful search: " << bold_off << average_successful_probes
                  << "\n";
        std::cout << bold_on << "average probes count of successful search evaluation: " << bold_off
                  << hash_table.successful_probes_evaluation()
                  << "\n\n";

        std::cout << bold_on << "total probes count of failed searches: " << bold_off << failed_probes_count << "\n";
        std::cout << bold_on << "average probes count of failed search: " << bold_off << average_failed_probes << "\n";
        std::cout << bold_on << "average probes count of failed search evaluation: " << bold_off
                  << hash_table.failed_probes_evaluation()
                  << "\n\n";
    }

    std::cout << bold_on << "total time of successful searches: " << bold_off << successful_duration.count() << "ms"
              << "\n";
    std::cout << bold_on << "average time of successful search: " << bold_off
              << successful_duration.count() / hash_table.fullness()
              << "ms" << "\n\n";

    std::cout << bold_on << "total time of failed searches: " << bold_off << failed_duration.count() << "ms" << "\n";
    std::cout << bold_on << "average time of failed search: " << bold_off
              << failed_duration.count() / (1'000'000 - hash_table.fullness()) << "ms" << "\n\n";
    std::cout << std::endl;
}

template<template<typename, typename, template<typename> typename, double> typename H, std::size_t min, bool count_average_probes>
void test_series(std::string_view title, std::unordered_set<std::size_t> random_numbers) {
    test<H, min, 0.1, count_average_probes>(title, random_numbers);
    test<H, min, 0.2, count_average_probes>(title, random_numbers);
    test<H, min, 0.3, count_average_probes>(title, random_numbers);
    test<H, min, 0.4, count_average_probes>(title, random_numbers);
    test<H, min, 0.5, count_average_probes>(title, random_numbers);
    test<H, min, 0.6, count_average_probes>(title, random_numbers);
    test<H, min, 0.7, count_average_probes>(title, random_numbers);
    test<H, min, 0.8, count_average_probes>(title, random_numbers);
    test<H, min, 0.9, count_average_probes>(title, random_numbers);
}

int main() {
//    auto random_numbers = generate_rand_numbers<1'000'000>(0, 1'000'000);
    auto random_numbers = generate_rand_numbers<15>(0, 1'000'000);

    test_deletion<DoubleHashingHashTable>("double hashing hash table", random_numbers);
    test_deletion<LinearHashingHashTable>("linear probing hash table", random_numbers);
    test_deletion<BucketHashTable>("bucket hash table", random_numbers);

//    test_series<DoubleHashingHashTable, 50'000, true>("double hashing hash table", random_numbers);
//    test_series<LinearHashingHashTable, 50'000, true>("linear probing hash table", random_numbers);
//    test_series<BucketHashTable, 50'000, false>("bucket hash table", random_numbers);

    return 0;
}
