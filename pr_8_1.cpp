/*
Реализуйте структуру данных типа “множество строк” на основе динамической
хеш-таблицы с открытой адресацией. Хранимые строки непустые и состоят из
строчных латинских букв. Хеш-функция строки должна быть реализована
с помощью вычисления значения многочлена методом Горнера. Начальный размер
таблицы должен быть равным 8-ми. Перехеширование выполняйте при добавлении
элементов в случае, когда коэффициент заполнения таблицы достигает 3/4.
Структура данных должна поддерживать операции добавления строки в множество,
удаления строки из множества и проверки принадлежности данной строки
множеству. 1_1. Для разрешения коллизий используйте квадратичное
пробирование. i-ая проба g(k, i)=g(k, i-1) + i (mod m). m - степень двойки.
1_2. Для разрешения коллизий используйте двойное хеширование.
Формат ввода

Каждая строка входных данных задает одну операцию над множеством.
Запись операции состоит из типа операции и следующей за ним через
пробел строки, над которой проводится операция. Тип операции – один
из трех символов:
+ означает добавление данной строки в множество;
- означает удаление строки из множества;
? означает проверку принадлежности данной строки множеству.
При добавлении элемента в множество НЕ ГАРАНТИРУЕТСЯ, что он
отсутствует в этом множестве. При удалении элемента из множества
НЕ ГАРАНТИРУЕТСЯ, что он присутствует в этом множестве.
Формат вывода

Программа должна вывести для каждой операции одну из двух строк OK или FAIL.
Для операции '?': OK, если элемент присутствует во множестве. FAIL иначе.
Для операции '+': FAIL, если добавляемый элемент уже присутствует
во множестве и потому не может быть добавлен. OK иначе.
Для операции '-': OK, если элемент присутствовал во множестве
и успешно удален. FAIL иначе.
*/


#include <iostream>
#include <string>
#include <stdexcept>
#include <cstring>

typedef unsigned char uc;

struct simple_str {
    int len;
    char* ptr;
};

class HashTable {
    private:
    int capacity;
    int free_slots;
    int del_slots;
    int in_place_bytes;
    char* buffer;
    void init(int capacity_bits, int in_place_bytes_);
    int Hash(simple_str key, int mod);
    int FindSlot(simple_str key);
    int Find(simple_str key);
    simple_str GetKey(int ind);
    bool Compare(simple_str key, int ind);
    void Rehash();

    public:
    HashTable();
    HashTable(int capacity_bits, int in_place_bytes_);
    ~HashTable();
    bool Insert(std::string key);
    bool Insert(simple_str key);
    bool Delete(std::string key);
    bool Delete(simple_str key);
    bool Has(std::string key);
    bool Has(simple_str key);
    void Flush();
};

void HashTable::init(int capacity_bits, int in_place_bytes_) {
    capacity = 1 << capacity_bits;
    free_slots = capacity;
    del_slots = 0;
    in_place_bytes = in_place_bytes_;
    if (in_place_bytes < 1 + sizeof(void*)) in_place_bytes = 1 + sizeof(void*);
    if (in_place_bytes >= 1 << 7) in_place_bytes = (1 << 7) - 1;
    buffer = (char*)calloc(capacity, in_place_bytes);
}

HashTable::HashTable() {
    init(3, 9);
}

HashTable::HashTable(int capacity_bits, int in_place_bytes_) {
    init(capacity_bits, in_place_bytes_);
}

HashTable::~HashTable() {
    for (int i = 0; i < capacity; i++) {
        char *ptr = buffer + i*in_place_bytes;
        if ((int)*ptr == 0xFF) {
            free(*((char**)(ptr + 1)));
        }
    }
    free(buffer);
}

int HashTable::Hash(simple_str key, int mod) {
    int res = 0;
    int mask = capacity - 1;
    for (int i = 0; i < key.len; i++) {
        res = (res*33 + (int)*(key.ptr + i)) & mask;
    }
    return res;
}

simple_str HashTable::GetKey(int ind) {
    char *ptr = buffer + ind*in_place_bytes;
    simple_str res;
    if (((uc)*ptr == 0x00) || ((uc)*ptr== 0xF0)) {
        res.len = 0;
        res.ptr = ptr++;
        return res;
    }
    if ((uc)*ptr == 0xFF) {
        ptr = *((char**)(ptr + 1));
        res.len = *((int*)ptr);
        ptr += sizeof(int);
    } else if (*ptr < in_place_bytes) {
        res.len = (int)*ptr;
        ptr++;
    } else {
        throw std::logic_error("Invalid entry, table is corrupt!");
    }
    res.ptr = ptr;
    return res;
}

// Assuming non-empty key
bool HashTable::Compare(simple_str key, int ind) {
    simple_str saved_key = GetKey(ind);
    if (saved_key.len != key.len) return false;
    bool res = true;
    for (int i = 0; i < key.len; i++) {
        res = res && (*(key.ptr + i) == *(saved_key.ptr + i));
    }
    return res;
}

// Assuming non-empty key
int HashTable::FindSlot(simple_str key) {
    int ind = Hash(key, capacity);
    for (int i = 0; i < capacity; i++) {
        ind = (ind + i) % capacity;
        if ((uc)*(buffer + in_place_bytes*ind) == 0x00) return ind;
        if (Compare(key, ind)) return -1;
    }
    throw std::logic_error("The table is full!");
}

void HashTable::Rehash() {
    int old_capacity = capacity;
    capacity = 2*(capacity - free_slots - del_slots) > capacity 
        ? capacity << 1 : capacity;
    free_slots = capacity;
    del_slots = 0;
    char *old_buffer = buffer;
    buffer = (char*)calloc(capacity, in_place_bytes);
    for (int i = 0; i < old_capacity; i++) {
        char *ptr = old_buffer + i*in_place_bytes;
        if (((uc)*ptr == 0x00) || ((uc)*ptr == 0xF0)) continue;
        if ((uc)*ptr == 0xFF) {
            ptr = *((char**)(ptr + 1));
            simple_str key = {*((int*)ptr), ptr + sizeof(int)};
            int ind = FindSlot(key);
            char *n_ptr = buffer + ind*in_place_bytes;
            *(n_ptr) = 0xFF;
            *((char**)(n_ptr + 1)) = ptr;
            free_slots--;
            continue;
        }
        if ((int)*ptr < in_place_bytes) {
            simple_str key = {(int)*ptr, ptr + 1};
            int ind = FindSlot(key);
            char *n_ptr = buffer + ind*in_place_bytes;
            *(n_ptr) = (char)key.len;
            std::memcpy(n_ptr + 1, ptr + 1, key.len);
            free_slots--;
            continue;
        }
        throw std::logic_error("Invalid entry, table is corrupt!");
    }
    free(old_buffer);
}

bool HashTable::Insert(std::string key) {
    simple_str s_key = {key.length(), (char*)key.data()};
    return Insert(s_key);
}

bool HashTable::Insert(simple_str key) {
    if (key.len < 1) return false;
    if (4*(capacity - free_slots) > 3*capacity) Rehash();
    int ind = FindSlot(key);
    if (ind < 0) return false;
    char *ptr = (buffer + in_place_bytes*ind);
    if (key.len < in_place_bytes) {
        *ptr = (char)key.len;
        std::memcpy(ptr+1, key.ptr, key.len);
        free_slots--;
        return true;
    }
    *ptr = 0xFF;
    char *external = (char*)malloc(sizeof(int) + key.len);
    *((char**)(ptr + 1)) = external;
    *((int*)external) = key.len;
    external += sizeof(int);
    std::memcpy(external, key.ptr, key.len);
    free_slots--;
    return true;
}

int HashTable::Find(simple_str key) {
    if (key.len < 1) return -1;
    int ind = Hash(key, capacity);
    for (int i = 0; i < capacity; i++) {
        if ((uc)*(buffer + in_place_bytes*ind) == 0x00) return -1;
        if (Compare(key, ind)) return ind;
        ind = (ind + i) % capacity;
    }
    return -1;
}

bool HashTable::Delete(std::string key) {
    simple_str s_key = {key.length(), (char*)key.data()};
    return Delete(s_key);
}

bool HashTable::Delete(simple_str key) {
    int ind = Find(key);
    if (ind < 0) return false;
    char *ptr = buffer + ind*in_place_bytes;
    if ((uc)*ptr == 0xFF) {
        free(*((char**)(ptr + 1)));
    }
    *ptr = 0xF0;
    del_slots++;
    return true;
}

bool HashTable::Has(std::string key) {
    simple_str s_key = {key.length(), (char*)key.data()};
    return Has(s_key);
}

bool HashTable::Has(simple_str key) {
    int ind = Find(key);
    if (ind < 0) return false;
    return true;
}

void listen() {
    HashTable table;
    std::string str;
    char command;
    while (std::cin >> command >> str) {
        switch (command) {
            case '+':
                std::cout << (table.Insert(str) ? "OK" : "FAIL") << "\n";
                break;
            case '-':
                std::cout << (table.Delete(str) ? "OK" : "FAIL") << "\n";
                break;
            case '?':
                std::cout << (table.Has(str) ? "OK" : "FAIL") << "\n";
                break;
        }
    }
}

int main() {
    listen();

    return 0;
}
