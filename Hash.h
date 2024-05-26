#ifndef UNTITLED3_HASH_H
#define UNTITLED3_HASH_H

#include <concepts>

template<template<typename> typename H, typename K>
concept Hash = requires(H<K> h, const K &k) {
    { H<K>{} } -> std::convertible_to<H<K>>;
    { h(k) } -> std::convertible_to<std::size_t>;
};

#endif //UNTITLED3_HASH_H
