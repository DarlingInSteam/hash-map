#ifndef UNTITLED3_LINEARPROBINGHASHTABLE_H
#define UNTITLED3_LINEARPROBINGHASHTABLE_H

#include <cstddef>
#include <memory>
#include <optional>
#include <cassert>
#include <complex>
#include "Hash.h"
#include "PrimeNumbers.h"

static constexpr double linear_default_load_factor_limit = 0.8;

template<typename K, typename V, template<typename> typename H, double load_factor_limit = linear_default_load_factor_limit> requires Hash<H, K>
class LinearHashingHashTable {
private:
    struct Node {
        explicit Node(std::pair<const K, V> kv) : kv(kv), isRemoved(false) {}

        std::pair<const K, V> kv;
        bool isRemoved;

        const K &key() {
            return kv.first;
        }

        V &value() {
            return kv.second;
        }
    };

public:
    explicit LinearHashingHashTable() : size_i(0),
                                        non_nullptr_size(0),
                                        non_removed_size(0),
                                        nodes(new Node *[prime_numbers[0]]),
                                        hash(H < K > {}) {
        std::fill(nodes, nodes + prime_numbers[0], nullptr);
    }

    bool insert(std::pair<const K, V> &&kv) {
        if (load_factor() >= load_factor_limit) {
            rehash();
        }

        return insert_without_rehash(std::move(kv));
    }

    std::optional<std::reference_wrapper<std::pair<const K, V>>> find(const K &key, std::size_t &probes_count) {
        std::size_t h1 = hash(key);
        std::size_t h = h1 % size();

        for (std::size_t i = 0; i < size(); ++i, h = (h1 + i) % size(), ++probes_count) {
            auto node = nodes[h];

            if (!node) {
                ++probes_count;
                return {};
            }

            if (!node->isRemoved && node->key() == key) {
                ++probes_count;
                return {std::ref(node->kv)};
            }
        }

        return {};
    }

    std::optional<std::pair<K, V>> remove(const K &key) {
        std::size_t h1 = hash(key);
        std::size_t h = h1 % size();

        for (std::size_t i = 0; i < size(); i++, h = (h1 + i) % size()) {
            auto node = nodes[h];

            if (node && !node->isRemoved && node->key() == key) {
                node->isRemoved = true;
                return {std::move(node->kv)};
            }
        }

        return {};
    }

    std::size_t fullness() {
        return non_removed_size;
    }

    inline std::size_t size() {
        return prime_numbers[size_i];
    }

    double successful_probes_evaluation() {
        auto alpha = static_cast<double>(fullness()) / static_cast<double>(size());
        return (1. / 2) * (1. + 1. / (1. - alpha));
    }

    double failed_probes_evaluation() {
        auto alpha = static_cast<double>(fullness()) / static_cast<double>(size());
        return (1. / 2) * (1. + 1. / std::pow(1 - alpha, 2));
    }

    inline double load_factor() {
        return static_cast<double>(non_removed_size) / static_cast<double>(size());
    }

    ~LinearHashingHashTable() {
        for (int i = 0; i < size(); i++) {
            delete nodes[i];
        }

        delete[] nodes;
    }

private:
    void rehash() {
        std::size_t old_size = size();
        size_i++;
        non_removed_size = 0;
        non_nullptr_size = 0;

        Node **buff = new Node *[size()];

        std::fill(buff, buff + size(), nullptr);
        std::swap(nodes, buff);

        for (int i = 0; i < old_size; i++) {
            auto node = buff[i];

            if (node && !node->isRemoved) {
                insert_without_rehash(std::move(node->kv));
            }

            delete node;
        }

        delete[] buff;
    }

    bool insert_without_rehash(std::pair<const K, V> &&kv) {
        std::size_t h1 = hash(kv.first);
        std::size_t h = h1 % size();

        for (std::size_t i = 0; i < size(); i++, h = (h1 + i) % size()) {
            auto node = nodes[h];

            if (node && !node->isRemoved && node->key() == kv.first) {
                return false;
            }

            if (!node) {
                nodes[h] = new Node(std::move(kv));

                ++non_removed_size;
                ++non_nullptr_size;
                return true;
            }

            if (node->isRemoved) {
                delete node;
                nodes[h] = new Node(std::move(kv));

                ++non_removed_size;
                return true;
            }
        }

        assert(true);
        return false;
    }

    inline std::size_t next_size() {
        return prime_numbers[size_i + 1];
    }

    std::size_t size_i;
    std::size_t non_nullptr_size;
    std::size_t non_removed_size;

    Node **nodes;
    H <K> hash;
};

#endif //UNTITLED3_LINEARPROBINGHASHTABLE_H
