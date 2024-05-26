#ifndef UNTITLED3_HASHER_H
#define UNTITLED3_HASHER_H

#include <concepts>

template<typename T, typename K>
concept Hash = requires(T t, const K &k) {
    { t(k) } -> std::convertible_to<std::size_t>;
};

#endif //UNTITLED3_HASHER_H
