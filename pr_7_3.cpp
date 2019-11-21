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


class Deque {
    private:
        int head;
        void **buffer;
        int tail;
        int capacity;
        void expand();
    public:
        Deque();
        ~Deque();
        int len();
        void push_back(void *num);
        void push_front(void *num);
        void *pop_back();
        void *pop_front();
        void *peek_back();
        void *peek_front();
};

Deque::Deque() {
    head = 0;
    tail = 0;
    capacity = 1 << 8;
    buffer = (void**)malloc(capacity*sizeof(void*));
}

Deque::~Deque() {
    free(buffer);
}

void Deque::expand() {
    void **new_buffer = (void**)malloc((capacity << 1)*sizeof(void*));
    int len = this->len();
    for (int i = 0; i < len; i++) {
        new_buffer[i] = buffer[(head + i)%capacity];
    }
    head = 0;
    tail = len;
    capacity = capacity << 1;
    free(buffer);
    buffer = new_buffer;
}

int Deque::len() {
    int len = (tail - head);
    if (len < 0) len += capacity;
    return len;
}

void Deque::push_back(void *num) {
    if (len() == capacity - 1) expand();
    buffer[tail] = num;
    tail = (tail + 1)%capacity;
}

void Deque::push_front(void *num) {
    if (len() == capacity - 1) expand();
    head--;
    if (head < 0) head += capacity;
    buffer[head] = num;
}

void *Deque::pop_back() {
    if (len() == 0) std::logic_error("Trying to pop an empty deque.");
    tail--;
    if (tail < 0) tail += capacity;
    return buffer[tail];
}

void *Deque::pop_front() {
    if (len() == 0) std::logic_error("Trying to pop an empty deque.");
    void *num = buffer[head];
    head = (head + 1)%capacity;
    return num;
}

void *Deque::peek_back() {
    if (len() == 0) std::logic_error("Trying to peek an empty deque.");
    int m_tail = tail - 1;
    if (m_tail < 0) m_tail += capacity;
    return buffer[m_tail];
}

void *Deque::peek_front() {
    if (len() == 0) std::logic_error("Trying to peek an empty deque.");
    return buffer[head];
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

class Node {
    public:
        int key;
        int priority;
        int rank;
        Node(int key, int priority);
        Node *parent;
        Node *left_child;
        Node *right_child;
        void link_to(Node *child);
        void unlink();
        Node** child(bool right);
};

Node::Node(int key, int priority) {
    parent = NULL;
    left_child = NULL;
    right_child = NULL;
    this->key = key;
    this->priority = priority;
    rank = 1;
}

Node** Node::child(bool right) {
    if (right) {
        return &right_child;
    } else {
        return &left_child;
    }
}

void Node::link_to(Node *new_parent) {
    if (parent != NULL)
        throw std::logic_error("Try to link a node that already has a parent!");
    parent = new_parent;
    Node **to_insert = (parent->child)(parent->key <= key);
    if (*to_insert != NULL)
        throw std::logic_error("Cannot link to an occupied node!");
    if (parent->priority < this->priority)
        throw std::logic_error("Cannot link to a lower priority node!");
    *to_insert = this;
    Node *current = parent;
    while (current != NULL) {
        current->rank += this->rank;
        current = current->parent;
    }
}

void Node::unlink() {
    if (parent != NULL) {
        Node **to_free = (parent->child)(parent->key <= key);
        if (*to_free != this)
            throw std::logic_error("Parent-child link is corrupt!");
        *to_free = NULL;
        Node *current = parent;
        while (current != NULL) {
            current->rank -= this->rank;
            current = current->parent;
        }
        parent = NULL;
    }
}




/* ------------------------------------------------------------------------- */


class CartTree {
    private:
        Node *root;
        CartTree split(int key);
        void merge(CartTree tree);
    public:
        CartTree();
        CartTree(Node *node);
        void insert(int key, int priority);
        void remove(int key);
        void wipe_nodes();
        Node* get_by_order(int order);
};

CartTree::CartTree() {
    root = NULL;
}

CartTree::CartTree(Node *node) {
    root = node;
}

void CartTree::wipe_nodes() {
    if (root == NULL) {
        return;
    }
    Deque stack;
    stack.push_back(root);
    while (stack.len() > 0) {
        Node *current = (Node*)stack.pop_back();
        Node *lc = current->left_child;
        Node *rc = current->right_child;
        delete current;
        if (rc != NULL) stack.push_back(rc);
        if (lc != NULL) stack.push_back(lc);
    }
}

// Nodes with keys equal to \key go to the left tree
CartTree CartTree::split(int key) {
    if (root == NULL) throw std::logic_error(
        "Trying to split an empty tree!");
    Node *second_root = root;
    while ((root->key > key) == (second_root->key > key)) {
        second_root = *((second_root->child)(root->key <= key));
        if (second_root == NULL) return CartTree();
    }
    Node *border_node = second_root->parent;
    second_root->unlink();
    CartTree second_tree = CartTree(second_root);
    CartTree third_tree = second_tree.split(key);
    if (third_tree.root != NULL) (third_tree.root)->link_to(border_node);
    return second_tree;
}

// Keys in \this are smaller than keys in \tree
void CartTree::merge(CartTree tree) {
    if (tree.root == NULL) return;
    if (root == NULL) {
        root = tree.root;
        return;
    }
    bool slide_right = root->priority >= (tree.root)->priority;
    Node *higher = slide_right ? root : tree.root;
    Node *lower = slide_right ? tree.root : root;
    root = higher;
    while (*(higher->child)(slide_right) != NULL) {
        if ((*(higher->child)(slide_right))->priority < lower->priority) break;
        higher = *(higher->child)(slide_right);
    }
    Node* child = *(higher->child)(slide_right);
    if (child != NULL) child->unlink();
    lower->link_to(higher);
    if (slide_right) {
        CartTree(child).merge(CartTree(lower));
    } else {
        CartTree(lower).merge(CartTree(child));
    }
}

void CartTree::insert(int key, int priority) {
    Node *new_node = new Node(key, priority);
    if (root == NULL) {
        root = new_node;
        return;
    }
    CartTree left_tree = split(key);
    CartTree right_tree = root->key > key ? *this : left_tree;
    left_tree =root->key > key ? left_tree : *this;
    Node *current = left_tree.root;
    if (current == NULL) {
        left_tree.root = new_node;
        left_tree.merge(right_tree);
        root = left_tree.root;
        return;
    }
    if (current->priority <= priority) {
        current->link_to(new_node);
        left_tree.root = new_node;
        left_tree.merge(right_tree);
        root = left_tree.root;
        return;
    }
    while (current->priority > priority) {
        if (current->right_child == NULL) {
            new_node->link_to(current);
            left_tree.merge(right_tree);
            return;
        }
        current = current->right_child;
    }
    Node* parent = current->parent;
    current->unlink();
    current->link_to(new_node);
    if (parent != NULL) new_node->link_to(parent);
    left_tree.merge(right_tree);
    return;
}

void CartTree::remove(int key) {
    if (root == NULL) return;
    CartTree left_tree = split(key);
    CartTree right_tree = root->key > key ? *this : left_tree;
    left_tree = root->key > key ? left_tree : *this;
    if (left_tree.root == NULL) return;
    Node *current = left_tree.root;
    while (current->key < key) {
        if (current->right_child == NULL) return;
        current = current->right_child;
    }
    if (current->key > key) return;
    Node *parent = current->parent;
    Node *lc = current->left_child;
    Node *rc = current->right_child;
    current->unlink();
    if (lc != NULL) lc->unlink();
    if (rc != NULL) rc->unlink();
    CartTree tree = CartTree(lc);
    tree.merge(CartTree(rc));
    if (parent == NULL) {
        left_tree.root = tree.root;
    } else {
        if (tree.root != NULL) (tree.root)->link_to(parent);
    }
    delete current;
    left_tree.merge(right_tree);
    root = left_tree.root;
}

Node* CartTree::get_by_order(int order) {
    if (root == NULL) return NULL;
    int root_rank = root->rank;
    if (order >= root_rank) return NULL;
    Node *current = root;
    int current_order = 0;
    if (current->left_child != NULL) current_order += current->left_child->rank;
    while (current_order != order) {
        if (current == NULL) throw std::logic_error(
            "Can't find element with valid order, tree is corrupt!");
        if (order > current_order) {
            current = current->right_child;
            if (current->left_child != NULL)
                current_order += current->left_child->rank;
            current_order++;
        } else {
            current = current->left_child;
            if (current->right_child != NULL)
                current_order -= current->right_child->rank;
            current_order--;
        }
    }
    return current;
}

void order_statistics(int *commands, int n_commands) {
    CartTree tree = CartTree();
    srand(time(0));

    for (int i = 0; i < n_commands; i++) {
        int key = commands[2*i];
        int order = commands[2*i + 1];
        if (key > 0) {
            tree.insert(key, rand());
        } else {
            tree.remove(-key);
        }
        std::cout << (tree.get_by_order(order))->key << " ";
    }
    tree.wipe_nodes();
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







