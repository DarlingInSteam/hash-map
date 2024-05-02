#include <iostream>
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
    HashTable(std::unique_ptr<Hasher>&& hasher) : hasher(std::move(hasher)), nodes(new Node*[default_size]), alive_size(0), nodes_size(default_size), alive_non_alive_size(0) {
        for (int i = 0; i < nodes_size; i++) {
            nodes[i] = nullptr;
        }
    };

    ~HashTable() {
        for (int i = 0; i < nodes_size; i++) {
            if (nodes[i]) delete nodes[i];
        }

        delete[] nodes;
    }

    bool add(std::string&& key, T&& value) {
        if (alive_size + 1 > int(nodes_size * resize_factor)) resize();
        else if (alive_non_alive_size > 2 * alive_size) rehash();

        int i = 0;
        int hash = hasher->calculate(key, nodes_size);
        int deletedHash = -1;

        while (i < nodes_size) {
            hash = (hash + i) % nodes_size;

            if (nodes[hash] != nullptr && nodes[hash]->value == value && !nodes[hash]->isDeleted) {
                return false;
            }
            if (nodes[hash] != nullptr && nodes[hash]->isDeleted && deletedHash == -1) {
                deletedHash = hash;
            }

            ++i;
        }

        if (deletedHash != -1) {
            nodes[deletedHash]->value = value;
            nodes[deletedHash]->isDeleted = false;
            nodes[deletedHash]->key = key;
            alive_size++;
            return true;
        }
        else {
            nodes[hash] = new Node();
            nodes[hash]->value = value;
            nodes[hash]->key = key;
            alive_size++;
            alive_non_alive_size++;
            return true;
        }
    }

    bool remove(const std::string& key) {
        int i = 0;
        int hash = hasher->calculate(key, nodes_size);

        while (i < nodes_size) {
            hash = (hash + i) % nodes_size;

            if (nodes[hash] != nullptr && !nodes[hash]->isDeleted && nodes[hash]->key == key) {
                nodes[hash]->isDeleted = true;
                alive_size--;
                return true;
            }

            ++i;
        }

        return false;
    }

    T* find(const std::string& key) {
        int i = 0;
        int hash = hasher->calculate(key, nodes_size);

        while (i < nodes_size) {
            hash = (hash + i) % nodes_size;

            if (nodes[hash] != nullptr && !nodes[hash]->isDeleted && nodes[hash]->key == key) {
                return &nodes[hash]->value;
            }

            ++i;
        }

        return nullptr;
    }

private:
    static const int default_size = 8;
    // u can change this value
    constexpr static const double resize_factor = 0.75;

    struct Node {
        T value;
        std::string key;
        bool isDeleted;
        Node() = default;
        Node(T&& value, std::string&& key): value(value), key(key) {}
    };

    std::unique_ptr<Hasher> hasher;
    Node** nodes;
    int alive_size;
    int nodes_size;
    int alive_non_alive_size;

    void rehash() {
        alive_non_alive_size = 0;
        alive_size = 0;

        Node** buff = new Node*[nodes_size];

        for (int i = 0; i < nodes_size; i++) {
            buff[i] = nullptr;
        }

        std::swap(nodes, buff);

        for (int i = 0; i < nodes_size; i++) {
            if (buff[i] != nullptr && buff[i]->isDeleted == false) {
                add(std::move(buff[i]->key), std::move(buff[i]->value));
            }
        }

        for (int i = 0; i < nodes_size; i++) {
            if (buff[i]) delete buff[i];
        }

        delete[] buff;
    }

    void resize() {
        int prev_nodes_size = nodes_size;
        nodes_size *= 2;
        alive_non_alive_size = 0;
        alive_size = 0;

        Node** buff = new Node*[nodes_size];

        for (int i = 0; i < nodes_size; i++) {
            buff[i] = nullptr;
        }

        std::swap(nodes, buff);

        for (int i = 0; i < prev_nodes_size; i++) {
            if (buff[i] != nullptr && buff[i]->isDeleted == false) {
                add(std::move(buff[i]->key), std::move(buff[i]->value));
            }
        }

        for (int i = 0; i < prev_nodes_size; i++) {
            if (buff[i]) delete buff[i];
        }

        delete[] buff;
    }
};

int main() {
    // Создаем хэш-таблицу с использованием простого хэшера
    std::unique_ptr<Hasher> simple_hasher = std::make_unique<SimpleHasher>();
    std::unique_ptr<HashTable<int>> hash_table = std::make_unique<HashTable<int>>(std::move(simple_hasher));

    // Тест на добавление элементов
    std::cout << "Add Test:\n";
    std::cout << "Adding key1: 10 - " << (hash_table->add("key1", 10) ? "Success" : "Failed") << std::endl;
    std::cout << "Adding key2: 20 - " << (hash_table->add("key2", 20) ? "Success" : "Failed") << std::endl;
    std::cout << "Adding key1: 30 - " << (hash_table->add("key1", 30) ? "Success" : "Failed") << std::endl; // Попытка добавить элемент с уже существующим ключом

    // Тест на удаление элементов
    std::cout << "\nRemove Test:\n";
    std::cout << "Removing key1 - " << (hash_table->remove("key1") ? "Success" : "Failed") << std::endl;
    std::cout << "Removing key3 - " << (hash_table->remove("key3") ? "Success" : "Failed") << std::endl; // Попытка удалить несуществующий элемент

    // Тест на поиск элементов
    std::cout << "\nFind Test:\n";
    int* found_value = hash_table->find("key2");
    if (found_value != nullptr) {
        std::cout << "Found key2: " << *found_value << std::endl;
    } else {
        std::cout << "Key2 not found." << std::endl;
    }
    found_value = hash_table->find("key3");
    if (found_value != nullptr) {
        std::cout << "Found key3: " << *found_value << std::endl;
    } else {
        std::cout << "Key3 not found." << std::endl;
    } // Поиск несуществующего элемента

    return 0;
}
