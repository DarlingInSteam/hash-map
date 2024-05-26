#ifndef UNTITLED3_BUCKETHASHTABLE_H
#define UNTITLED3_BUCKETHASHTABLE_H

#include <cstddef>
#include <memory>
#include <optional>
#include <iostream>
#include <cassert>
#include "Hash.h"
#include "PrimeNumbers.h"

static constexpr double bucket_default_load_factor_limit = 0.8;

template<typename K, typename V, template<typename> typename H, double load_factor_limit = bucket_default_load_factor_limit> requires Hash<H, K>
class BucketHashTable {
private:
    struct Node {
        std::pair<const K, V> kv;
        Node *next;

        explicit Node(std::pair<const K, V> &&kv) : kv(kv), next(nullptr) {}
    };

public:
    BucketHashTable() : size_i(0),
                        fullness_size(0),
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
        std::size_t h = hash(key) % size();
        Node *node = nodes[h];

        while (node != nullptr) {
            ++probes_count;

            if (node->kv.first == key) {
                return {std::ref(node->kv)};
            }

            node = node->next;
        }

        return {};
    }

    std::optional<std::pair<K, V>> remove(const K &key) {
        std::size_t h = hash(key) % size();
        Node *node = nodes[h];
        Node *prev = nullptr;

        while (node != nullptr) {
            if (node->kv.first == key) {
                std::pair<K, V> kv = std::move(node->kv);

                if (prev == nullptr) {
                    nodes[h] = node->next;
                } else {
                    prev->next = node->next;
                }

                delete node;
                --fullness_size;

                return {std::move(kv)};
            }

            prev = node;
            node = node->next;
        }

        return {};
    }

    inline double load_factor() const {
        return static_cast<double>(fullness()) / static_cast<double>(size());
    }

    inline std::size_t fullness() const {
        return fullness_size;
    }

    inline std::size_t size() const {
        return prime_numbers[size_i];
    }

//    double successful_probes_evaluation() {
//        return 0;
//    }
//
//    double failed_probes_evaluation() {
//        return 0;
//    }

    ~BucketHashTable() {
        for (std::size_t i = 0; i < size(); ++i) {
            Node *node = nodes[i];
            while (node != nullptr) {
                Node *temp = node;
                node = node->next;
                delete temp;
            }
        }
        delete[] nodes;
    }

private:
    void rehash() {
        std::size_t old_size = size();
        size_i++;
        fullness_size = 0;

        Node **buff = new Node *[size()];
        std::fill(buff, buff + size(), nullptr);

        std::swap(buff, nodes);

        for (std::size_t i = 0; i < old_size; ++i) {
            Node *node = buff[i];

            while (node != nullptr) {
                insert_without_rehash(std::move(node->kv));

                Node *node_to_delete = node;
                node = node->next;

                delete node_to_delete;
            }
        }

        delete[] buff;
    }

    bool insert_without_rehash(std::pair<const K, V> &&kv) {
        std::size_t h = hash(kv.first) % size();
        Node *node = nodes[h];

        while (node != nullptr) {
            if (node->kv.first == kv.first) {
                return false;
            }
            node = node->next;
        }

        Node *new_node = new Node(std::move(kv));
        new_node->next = nodes[h];
        nodes[h] = new_node;

        ++fullness_size;
        return true;
    }

    std::size_t size_i;
    std::size_t fullness_size;

    Node **nodes;

    H <K> hash;
};

#endif //UNTITLED3_BUCKETHASHTABLE_H
