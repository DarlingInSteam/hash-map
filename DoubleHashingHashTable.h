#ifndef UNTITLED3_OPENADDRESSINGHASHTABLE_H
#define UNTITLED3_OPENADDRESSINGHASHTABLE_H

#include <cstddef>
#include <memory>
#include <optional>


template<typename K, typename V, Hasher<K> H>
class OpenAddressingHashTable {
private:
    struct Node {
        Node(K &&key, V &&value) : kv(key, value), isRemoved(false) {}

        std::pair<K, V> kv;
        bool isRemoved;
    };

    static constexpr std::size_t prime_numbers_count = 26;
    static constexpr auto prime_numbers = std::array<std::size_t, prime_numbers_count>{
            53, 97, 193, 389, 769, 1543, 3079, 6151, 12289, 24593, 49157, 98317, 196613, 393241, 786433, 1572869,
            3145739, 6291469, 12582917, 25165843, 50331653, 100663319, 201326611, 402653189, 805306457, 1610612741
    };

    static constexpr double load_factor_limit = 0.5;

public:
    explicit OpenAddressingHashTable(H &&hasher) : size_i(0),
                                                   non_nullptr_size(0),
                                                   non_removed_size(0),
                                                   nodes(new Node *[prime_numbers[0]]),
                                                   hasher(hasher) {
        std::fill(nodes, nodes + prime_numbers[0], nullptr);
    }

    bool add(K &&key, V &&value) {
        if (load_factor() >= load_factor_limit) {
            rehash();
        }

        return insert_without_rehash(key, value);
    }

    std::optional<std::pair<const K &, V &>> find(const K &key) {
        std::size_t h1 = hasher(key);
        std::size_t h2 = 1 + h1 % (size() - 1);
        std::size_t hash = h1 % size();

        for (std::size_t i = 0; i < size(); i++, hash = (h1 + i * h2) % size()) {
            auto node = nodes[hash];

            if (node && !node->isRemoved && node->key == key) {
                const K &k = node->kv->first;
                V &v = node->kv->second;
                return {{k, v}};
            }
        }

        return {};
    }

    std::optional<std::pair<K, V>> remove(const K &key) {
        std::size_t h1 = hasher(key);
        std::size_t h2 = 1 + h1 % (size() - 1);
        std::size_t hash = h1 % size();

        for (std::size_t i = 0; i < size(); i++, hash = (h1 + i * h2) % size()) {
            auto node = nodes[hash];

            if (node && !node->isRemoved && node->key == key) {
                node->isRemoved = true;
                return {std::move(node->kv)};
            }
        }

        return {};
    }

    ~OpenAddressingHashTable() {
        for (int i = 0; i < size(); i++) {
            delete nodes[i];
        }

        delete[] nodes;
    }

private:
    void rehash() {
        std::size_t new_size = next_size();
        Node *buff = new Node *[new_size];

        std::fill(buff, buff + new_size, nullptr);
        std::swap(nodes, buff);

        for (int i = 0; i < size(); i++) {
            auto node = nodes[i];

            if (node && !node->isRemoved) {
                insert_without_rehash(std::move(node->key), std::move(node->value));
            }

            delete node;
        }

        delete[] buff;
    }

    bool insert_without_rehash(K &&key, V &&value) {
        std::size_t h1 = hasher(key);
        std::size_t h2 = 1 + h1 % (size() - 1);
        std::size_t hash = h1 % size();

        bool new_pos_found = false;
        std::size_t new_pos_hash = 0;

        for (std::size_t i = 0; i < size(); i++, hash = (h1 + i * h2) % size()) {
            auto node = nodes[hash];

            if (!node && node->key == key) {
                return false;
            }

            if (!new_pos_found && (!node || node->isRemoved)) {
                new_pos_found = true;
                new_pos_hash = hash;
                continue;
            }
        }

        if (!new_pos_found) {
            // TODO
            return false;
        }

        if (auto node = &nodes[new_pos_hash]; node) {
            node->isRemoved = false;
            node->key = key;
            node->value = value;
        } else {
            nodes[new_pos_hash] = new Node(key, value);
            ++non_nullptr_size;
        }
        ++non_removed_size;

        return true;
    }

    inline double load_factor() {
        return static_cast<double>(non_removed_size) / static_cast<double>(size());
    }

    inline std::size_t size() {
        return prime_numbers[size_i];
    }

    inline std::size_t next_size() {
        prime_numbers[size_i + 1];
    }

    std::size_t size_i;
    std::size_t non_nullptr_size;
    std::size_t non_removed_size;

    Node **nodes;
    H hasher;

};

#endif // UNTITLED3_OPENADDRESSINGHASHTABLE_H