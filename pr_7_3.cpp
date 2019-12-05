// Множество натуральных чисел постоянно меняется. Элементы в нем добавляются
// и удаляются по одному. Вычислите указанные порядковые статистики после
// каждого добавления или удаления.
// Требуемая скорость выполнения запроса - O(log n) в среднем.
// В реализации используйте декартовы деревья.
// Формат ввода

// Дано число N и N строк. (1 ≤ N ≤ 10000)
// Каждая строка содержит команду добавления или удаления
// натуральных чисел (от 1 до 109),
// а также запрос на получение k-ой порядковой статистики (0 ≤ k < N).
// Команда добавления числа A задается положительным числом A, команда удаления числа A
// задается отрицательным числом “-A”.
// Формат вывода

// N строк. В каждой строке - текущая k-ая порядковая статистика из запроса.


#include <iostream>
#include <stdexcept>
#include <ctime>
#include <cstring>

template <typename T>
class Deque {
    private:
        int head;
        T *buffer;
        int tail;
        int capacity;
        void expand();
    public:
        Deque();
        ~Deque();
        Deque(const Deque<T> &source);
        Deque(Deque<T> &&source) = delete;
        Deque& operator=(const Deque<T> &source);
        Deque& operator=(Deque<T> &&source) = delete;
        int len() const;
        void push_back(T item);
        void push_front(T item);
        T pop_back();
        T pop_front();
        T peek_back();
        T peek_front();
};

template <typename T>
Deque<T>::Deque() {
    head = 0;
    tail = 0;
    capacity = 1 << 8;
    buffer = (T*)malloc(capacity*sizeof(T));
}

template <typename T>
Deque<T>::~Deque() {
    free(buffer);
}

template <typename T>
Deque<T>::Deque(const Deque<T> &source) {
    head = source.head;
    tail = source.tail;
    capacity = source.capacity;
    buffer = (T*)malloc(capacity*sizeof(T));
    memcpy(buffer, source.buffer, capacity*sizeof(T));
}

template <typename T>
Deque<T>& Deque<T>::operator= (const Deque<T> &source) {
    head = source.head;
    tail = source.tail;
    capacity = source.capacity;
    free(buffer);
    buffer = (T*)malloc(capacity*sizeof(T));
    memcpy(buffer, source.buffer, capacity*sizeof(T));
}

template <typename T>
void Deque<T>::expand() {
    T *new_buffer = (T*)malloc((capacity << 1)*sizeof(T));
    const int len = this->len();
    if (head <= tail) {
        memcpy(new_buffer, buffer + head, len*sizeof(T));
    } else {
        memcpy(new_buffer, buffer + head, (capacity - head)*sizeof(T));
        memcpy(new_buffer + (capacity - head), buffer, tail*sizeof(T));
    }
    head = 0;
    tail = len;
    capacity = capacity << 1;
    free(buffer);
    buffer = new_buffer;
}

template <typename T>
int Deque<T>::len() const {
    int len = (tail - head);
    if (len < 0) len += capacity;
    return len;
}

template <typename T>
void Deque<T>::push_back(T item) {
    if (len() == capacity - 1) expand();
    buffer[tail] = item;
    tail = (tail + 1)%capacity;
}

template <typename T>
void Deque<T>::push_front(T item) {
    if (len() == capacity - 1) expand();
    head--;
    if (head < 0) head += capacity;
    buffer[head] = item;
}

template <typename T>
T Deque<T>::pop_back() {
    if (len() == 0) std::logic_error("Trying to pop an empty deque.");
    tail--;
    if (tail < 0) tail += capacity;
    return buffer[tail];
}

template <typename T>
T Deque<T>::pop_front() {
    if (len() == 0) std::logic_error("Trying to pop an empty deque.");
    T item = buffer[head];
    head = (head + 1)%capacity;
    return item;
}

template <typename T>
T Deque<T>::peek_back() {
    if (len() == 0) std::logic_error("Trying to peek an empty deque.");
    int m_tail = tail - 1;
    if (m_tail < 0) m_tail += capacity;
    return buffer[m_tail];
}

template <typename T>
T Deque<T>::peek_front() {
    if (len() == 0) std::logic_error("Trying to peek an empty deque.");
    return buffer[head];
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

struct Node {
    int key;
    int priority;
    int rank;
    Node *left_child;
    Node *right_child;
    Node(): key(0), priority(0), rank(1), left_child(NULL), right_child(NULL) {};
};

class CartTree {
    private:
    Node *root;

    bool gt(int x, int y, bool eq);
    Node **GetChild(Node *node, bool right);
    Node *DropChild(Node *node, bool right);
    Node *Split(int key, bool to_right = false);
    void Merge(Node *sub_root);


    public:
    void Insert(int key);
    void Delete(int key);
    Node *GetByOrder(int order);
    CartTree();
    ~CartTree();
    CartTree(const CartTree &source);
    CartTree(CartTree &&source) = delete;
    CartTree& operator=(const CartTree &source);
    CartTree& operator=(CartTree &&soruce) = delete;
};

CartTree::CartTree() {
    root = NULL;
    srand(time(0));
}

CartTree::~CartTree() {
    if (!root) return;
    Deque<Node*> stack;
    stack.push_back(root);
    while (stack.len() > 0) {
        Node *current = stack.pop_back();
        Node *lc = current->left_child;
        Node *rc = current->right_child;
        delete current;
        if (rc) stack.push_back(rc);
        if (lc) stack.push_back(lc);
    }
}

CartTree::CartTree(const CartTree &source) {
    if (!source.root) {
        root = NULL;
        return;
    }
    root = new Node;
    *root = *(source.root);
    Deque<Node*> stack;
    stack.push_back(root);
    while (stack.len() > 0) {
        Node *current = stack.pop_back();
        Node *lc = current->left_child;
        Node *rc = current->right_child;
        if (rc) {
            Node *n_rc = new Node;
            *n_rc = *rc;
            current->right_child = n_rc;
            stack.push_back(n_rc);
        }
        if (lc) {
            Node *n_lc = new Node;
            *n_lc = *lc;
            current->left_child = n_lc;
            stack.push_back(n_lc);
        }
    }
}

CartTree& CartTree::operator= (const CartTree &source) {
    if (root) {
        Deque<Node*> stack;
        stack.push_back(root);
        while (stack.len() > 0) {
            Node *current = stack.pop_back();
            Node *lc = current->left_child;
            Node *rc = current->right_child;
            delete current;
            if (rc) stack.push_back(rc);
            if (lc) stack.push_back(lc);
        }
    }
    if (!source.root)  {
        root = NULL;
        return *this;
    }
    root = new Node;
    *root = *(source.root);
    Deque<Node*> stack;
    stack.push_back(root);
    while (stack.len() > 0) {
        Node *current = stack.pop_back();
        Node *lc = current->left_child;
        Node *rc = current->right_child;
        if (rc) {
            Node *n_rc = new Node;
            *n_rc = *rc;
            current->right_child = n_rc;
            stack.push_back(n_rc);
        }
        if (lc) {
            Node *n_lc = new Node;
            *n_lc = *lc;
            current->left_child = n_lc;
            stack.push_back(n_lc);
        }
    }
    return *this;    
}

Node **CartTree::GetChild(Node *node, bool right) {
    if (!node) return NULL;
    if (right) return &(node->right_child);
    return &(node->left_child);
}

Node *CartTree::DropChild(Node *node, bool right) {
    return *GetChild(node, right);
}

bool CartTree::gt(int x, int y, bool eq) {
    return !eq ? x > y : x >= y;
}

Node *CartTree::Split(int key, bool to_right) {
    if (!root) return NULL;
    Node *sub_root = root;
    Deque<Node*> stack;
    while (gt(root->key, key, to_right) == gt(sub_root->key, key, to_right)) {
        stack.push_back(sub_root);
        sub_root = *GetChild(sub_root, !gt(root->key, key, to_right));
        if (!sub_root) return NULL;
    }
    Node *border = stack.peek_back();
    Node *orig_root = root;
    root = sub_root;
    Node *sub_sub_root = Split(key, to_right);
    root = orig_root;
    *GetChild(border, !gt(root->key, key, to_right)) = sub_sub_root;
    while (stack.len() > 0) (stack.pop_back())->rank -= sub_root->rank;
    return sub_root;
}

// Assume that all keys in one tree are not smaller than keys in another tree
void CartTree::Merge(Node *sub_root) {
    if (!sub_root) return;
    if (!root) {
        root = sub_root;
        return;
    }
    Node *higher = root->priority > sub_root->priority ? root : sub_root;
    Node *lower = root->priority > sub_root->priority ? sub_root : root;
    root = higher;
    bool right_descent = higher->key <= lower->key;
    Deque<Node*> stack;
    Node *sub_sub_root = higher;
    while (sub_sub_root) {
        stack.push_back(sub_sub_root);
        sub_sub_root = *GetChild(sub_sub_root, right_descent);
        if (!sub_sub_root) break;
        if (sub_sub_root->priority < lower->priority) break;
    }
    int rank_delta = 0;
    if (sub_sub_root) rank_delta -= sub_sub_root->rank;
    higher = stack.peek_back();
    *GetChild(higher, right_descent) = lower;
    Node* orig_root = root;
    root = lower;
    Merge(sub_sub_root);
    root = orig_root;
    rank_delta += lower->rank;
    while (stack.len() > 0) (stack.pop_back())->rank += rank_delta;
}

void CartTree::Insert(int key) {
    Node *new_node = new Node;
    new_node->key = key;
    new_node->priority = rand();
    if (!root) {
        root = new_node;
        return;
    }
    Node *sub_root = Split(key);
    if (root->key > key) {
        Node *temp = root;
        root = sub_root;
        sub_root = temp;
    }
    if (!root) {
        root = new_node;
        Merge(sub_root);
        return;
    }
    Node *sub_sub_root = Split(key, true);
    if (root->key < key) {
        Node *temp = root;
        root = sub_sub_root;
        sub_sub_root = temp;
    }
    Merge(new_node);
    Merge(sub_sub_root);
    Merge(sub_root);    
}

void CartTree::Delete(int key) {
    if (!root) return;
    Node *sub_root = Split(key);
    if (root->key > key) {
        Node *temp = root;
        root = sub_root;
        sub_root = temp;
    }
    if (!root) {
        root = sub_root;
        return;
    }
    Node *sub_sub_root = Split(key, true);
    if (root->key < key) {
        Node *temp = root;
        root = sub_sub_root;
        sub_sub_root = temp;
    }
    if (root) {
        Node *temp = root;
        root = root->right_child;
        delete temp;
    }
    Merge(sub_sub_root);
    Merge(sub_root);
}

Node* CartTree::GetByOrder(int order) {
    if (!root) return NULL;
    if (order >= root->rank) return NULL;
    Node *current = root;
    int current_order = current->left_child ? current->left_child->rank : 0;
    while (order != current_order) {
        if (!current) throw std::logic_error(
            "Can't find element with valid order, tree is corrupt!");
        if (order > current_order) {
            current = current->right_child;
            current_order++;
            if (current->left_child) current_order += current->left_child->rank;
        }
        if (order < current_order) {
            current = current->left_child;
            current_order--;
            if (current->right_child) current_order -= current->right_child->rank;
        }
    }
    return current;
}

void order_statistics(int *commands, int n_commands) {
    CartTree tree;
    for (int i = 0; i < n_commands; i++) {
        int key = commands[2*i];
        int order = commands[2*i + 1];
        if (key > 0) {
            tree.Insert(key);
        } else {
            tree.Delete(-key);
        }
        std::cout << (tree.GetByOrder(order))->key << " ";
    }
    std::cout << std::endl;
}


int main() {
    int n_commands;
    std::cin >> n_commands;

    int* commands = (int*)malloc(2*n_commands*sizeof(int));
    for (int i = 0; i < 2*n_commands; i++) {
        std::cin >> commands[i];
    }

    order_statistics(commands, n_commands);
    free(commands);
    return 0;
}












