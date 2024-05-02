#include <iostream>
#include <vector>
#include <memory>

class Hasher {
public:
    virtual int calculate(const std::string& key, int nodes_size) = 0;
    virtual ~Hasher() = default;
};

class SimpleHasher : public Hasher {
public:
    int calculate(const std::string& key, int nodes_size) override {
        int hash = 0;
        for (char c : key) {
            hash += c;
        }
        return hash % nodes_size;
    }
};

template <typename T>
class HashTable {
public:
    HashTable(std::unique_ptr<Hasher>&& hasher) : hasher(std::move(hasher)), alive_size(0), nodes_size(default_size), alive_non_alive_size(0) {
        nodes = new std::vector<Node*>[nodes_size];
    };

    ~HashTable() {
        // Clean up nodes
        for (int i = 0; i < nodes_size; ++i) {
            for (auto& node : nodes[i]) {
                delete node;
            }
        }
        delete[] nodes;
    }

    bool add(std::string&& key, T&& value) {
        if (alive_size + 1 > int(nodes_size * resize_factor)) resize();
        else if (alive_non_alive_size > 2 * alive_size) rehash();

        int hash = hasher->calculate(key, nodes_size);

        for (auto& node : nodes[hash]) {
            if (node->key == key && !node->isDeleted) {
                return false; // Key already exists
            }
        }

        nodes[hash].emplace_back(new Node(std::move(key), std::move(value)));
        alive_size++;
        alive_non_alive_size++;
        return true;
    }

    bool remove(const std::string& key) {
        int hash = hasher->calculate(key, nodes_size);

        for (auto it = nodes[hash].begin(); it != nodes[hash].end(); ++it) {
            if ((*it)->key == key && !(*it)->isDeleted) {
                (*it)->isDeleted = true;
                alive_size--;
                return true;
            }
        }

        return false; // Key not found
    }

    T* find(const std::string& key) {
        int hash = hasher->calculate(key, nodes_size);

        for (auto& node : nodes[hash]) {
            if (node->key == key && !node->isDeleted) {
                return &(node->value);
            }
        }

        return nullptr; // Key not found
    }

private:
    static const int default_size = 8;
    constexpr static const double resize_factor = 0.75;

    struct Node {
        T value;
        std::string key;
        bool isDeleted;
        Node(std::string&& key, T&& value) : key(std::move(key)), value(std::move(value)), isDeleted(false) {}
    };

    std::unique_ptr<Hasher> hasher;
    std::vector<Node*>* nodes;
    int alive_size;
    int nodes_size;
    int alive_non_alive_size;

    void rehash() {
        alive_non_alive_size = 0;
        alive_size = 0;
        int new_nodes_size = nodes_size * 2;
        std::vector<Node*>* new_nodes = new std::vector<Node*>[new_nodes_size];

        for (int i = 0; i < nodes_size; ++i) {
            for (auto& node : nodes[i]) {
                if (!node->isDeleted) {
                    int hash = hasher->calculate(node->key, new_nodes_size);
                    new_nodes[hash].emplace_back(node);
                    alive_size++;
                }
                delete node;
            }
        }

        nodes_size = new_nodes_size;
        delete[] nodes;
        nodes = new_nodes;
    }

    void resize() {
        int prev_nodes_size = nodes_size;
        nodes_size *= 2;
        alive_non_alive_size = 0;
        alive_size = 0;
        std::vector<Node*>* new_nodes = new std::vector<Node*>[nodes_size];

        for (int i = 0; i < prev_nodes_size; ++i) {
            for (auto& node : nodes[i]) {
                if (!node->isDeleted) {
                    int hash = hasher->calculate(node->key, nodes_size);
                    new_nodes[hash].emplace_back(node);
                    alive_size++;
                }
                delete node;
            }
        }

        delete[] nodes;
        nodes = new_nodes;
    }
};

int main() {
    std::unique_ptr<Hasher> simple_hasher = std::make_unique<SimpleHasher>();
    HashTable<int> hash_table(std::move(simple_hasher));

    // Add some elements
    hash_table.add("key1", 10);
    hash_table.add("key2", 20);
    hash_table.add("key1", 30); // Attempt to add duplicate key

    // Remove an element
    hash_table.remove("key3");

    // Find elements
    int* found_value = hash_table.find("key1");
    if (found_value != nullptr) {
        std::cout << "Found key2: " << *found_value << std::endl;
    } else {
        std::cout << "Key2 not found." << std::endl;
    }

    return 0;
}
