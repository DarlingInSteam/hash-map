//
// Created by user on 5/26/24.
//

#ifndef UNTITLED3_PRIMENUMBERS_H
#define UNTITLED3_PRIMENUMBERS_H

#include <array>

constexpr std::size_t prime_numbers_count = 26;
constexpr auto prime_numbers = std::array<std::size_t, prime_numbers_count>{
        53, 97, 193, 389, 769, 1543, 3079, 6151, 12289, 24593, 49157, 98317, 196613, 393241, 786433, 1572869,
        3145739, 6291469, 12582917, 25165843, 50331653, 100663319, 201326611, 402653189, 805306457, 1610612741
};

#endif //UNTITLED3_PRIMENUMBERS_H
