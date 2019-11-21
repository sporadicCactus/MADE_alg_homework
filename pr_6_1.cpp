// Дано число N < 106 и последовательность целых чисел из [-231..231] длиной N.
// Требуется построить бинарное дерево, заданное наивным порядком вставки.
// Т.е., при добавлении очередного числа K в дерево с корнем root,
// если root→Key ≤ K, то узел K добавляется в правое поддерево root; иначе
// в левое поддерево root. Выведите элементы в порядке pre-order (сверху вниз).
// Рекурсия запрещена.


#include <iostream>
#include <stdexcept>

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
    if (len() == 0) throw std::logic_error("Trying to pop an empty deque.");
    tail--;
    if (tail < 0) tail += capacity;
    return buffer[tail];
}

void *Deque::pop_front() {
    if (len() == 0) throw std::logic_error("Trying to pop an empty deque.");
    void *num = buffer[head];
    head = (head + 1)%capacity;
    return num;
}

void *Deque::peek_back() {
    if (len() == 0) throw std::logic_error("Trying to peek an empty deque.");
    int m_tail = tail - 1;
    if (m_tail < 0) m_tail += capacity;
    return buffer[m_tail];
}

void *Deque::peek_front() {
    if (len() == 0) throw std::logic_error("Trying to peek an empty deque.");
    return buffer[head];
}

/* --------------------------------------------------------- */

class Node {
    public:
        int key;
        Node *parent;
        Node *left_child;
        Node *right_child;
        void link_to(Node *child);
        void unlink();
        Node** child(bool right);
        Node(int num);
};

Node** Node::child(bool right) {
    if (right) {
        return &right_child;
    } else {
        return &left_child;
    }
}

void Node::link_to(Node *new_parent) {
    parent = new_parent;
    Node **to_insert = (parent->child)(parent->key <= key);
    if (*to_insert != NULL)
        throw std::logic_error("Cannot link to an occupied node!");
    *to_insert = this;
}

void Node::unlink() {
    if (parent != NULL) {
        Node **to_free = (parent->child)(parent->key <= key);
        if (*to_free != this)
            throw std::logic_error("Parent-child link is corrupt!");
        *to_free = NULL;
        parent = NULL;
    }
}

Node::Node(int num) {
    parent = NULL;
    left_child = NULL;
    right_child = NULL;
    key = num;
}

/* --------------------------------------------------------- */

class BTree {
    private:
        Node *root;
        void insert(Node* node);
        Node* remove_root();
        void remove(Node* node);
    public:
        BTree();
        BTree(Node *node);
        void insert(int num);
        void traverse_inorder(void (*callback)(Node*, void*), void* arg);
        void traverse_preorder(void (*callback)(Node*, void*), void* arg);
        void traverse_postorder(void (*callback)(Node*, void*), void* arg);
        void traverse_level(void (*callback)(Node*, void*), void* arg);
};

BTree::BTree() {
    root = NULL;
}

BTree::BTree(Node *node) {
    root = node;
}

void BTree::insert(Node* node) {
    if (root == NULL) {
        root = node;
        return;
    }

    Node* current = root;
    while (current != node) {
        Node *child = *((current->child)(node->key >= current->key));
        if (child == NULL) {
            node->link_to(current);
            current = node;
        } else {
            current = child;
        }
    }
}

void BTree::insert(int num) {
    Node *node = new Node(num);
    insert(node);
}

Node* BTree::remove_root() {
    if (root->parent != NULL)
        throw std::logic_error("Cannot remove root from a non-free tree!");

    Node *old_root = root;
    Node *new_root = root->right_child;
    bool right_side = true;

    if (new_root == NULL) {
        new_root = root->left_child;
        right_side = false;
    }
    if (new_root == NULL) {
        root = NULL;
        return old_root;
    }

    while (*(new_root->child(!right_side)) != NULL) {
        new_root = *(new_root->child(!right_side));
    }

    new_root->unlink();
    if (*(new_root->child)(right_side) != NULL) {
        (*(new_root->child)(right_side))->unlink();
        (*(new_root->child)(right_side))->link_to(new_root->parent);
    }

    if (root->left_child != NULL) {
        (root->left_child)->unlink();
        (root->left_child)->link_to(new_root);
    }

    if (root->right_child != NULL) {
        (root->left_child)->unlink();
        (root->right_child)->link_to(new_root);
    }

    root = new_root;
    return old_root;
}

void BTree::remove(Node *node) {
    Node *parent = node->parent;
    node->unlink();
    BTree node_tree = BTree(node);
    node_tree.remove_root();
    (node_tree.root)->link_to(parent);
}

void BTree::traverse_inorder(void (*callback)(Node*, void*), void* arg) {
    if (root == NULL) {
        return;
    }
    Deque stack;
    stack.push_back(root);
    bool going_down = true;
    while (stack.len() > 0) {
        Node *current = (Node*)stack.peek_back();
        Node *lc = current->left_child;
        Node *rc = current->right_child;
        if ((!going_down) || (lc == NULL)) {
            (*callback)(current, arg);
            going_down = false;
            stack.pop_back();
            if (rc != NULL) {going_down = true; stack.push_back(rc);}
            continue;
        }
        stack.push_back(lc);
    }
}

void BTree::traverse_preorder(void (*callback)(Node*, void*), void* arg) {
    if (root == NULL) {
        return;
    }
    Deque stack;
    stack.push_back(root);
    while (stack.len() > 0) {
        Node *current = (Node*)stack.pop_back();
        Node *lc = current->left_child;
        Node *rc = current->right_child;
        (*callback)(current, arg);
        if (rc != NULL) stack.push_back(rc);
        if (lc != NULL) stack.push_back(lc);
    }
}

void BTree::traverse_postorder(void (*callback)(Node*, void*), void* arg) {
    if (root == NULL) {
        return;
    }
    Deque stack;
    Deque sec_stack;
    stack.push_back(root);
    sec_stack.push_back(NULL);
    bool going_down = true;
    while (stack.len() > 0) {
        Node *current = (Node*)stack.peek_back();
        Node *lc = current->left_child;
        Node *rc = current->right_child;
        if (going_down) {
            if (lc != NULL) {
                stack.push_back(lc);
                continue;
            }
            if (rc !=  NULL) {
                stack.push_back(rc);
                sec_stack.push_back(current);
                continue;
            }
        } else {
            if (rc != NULL) {
                if (sec_stack.peek_back() != current) {
                    sec_stack.push_back(current);
                    stack.push_back(rc);
                    going_down = true;
                    continue;
                }
                sec_stack.pop_back();
            }
        }
        going_down = false;
        (*callback)(current, arg);
        stack.pop_back();
    }
}

void BTree::traverse_level(void (*callback)(Node*, void*), void* arg) {
    if (root == NULL) {
        return;
    }
    Deque queue;
    queue.push_back(root);
    while (queue.len() > 0) {
        Node *current = (Node*)queue.pop_front();
        Node *lc = current->left_child;
        Node *rc = current->right_child;
        if (lc != NULL) queue.push_back(lc);
        if (rc != NULL) queue.push_back(rc);

        (*callback)(current, arg);
    }
}

void print_key_and_delete(Node *node, void* ) {
    std::cout << node->key << " ";
    delete node;
}

int main() {
    int n_nodes;
    std::cin >> n_nodes;

    BTree tree = BTree();
    for (int i = 0; i < n_nodes; i++) {
        int key;
        std::cin >> key;
        tree.insert(key);
    }

    tree.traverse_preorder(&print_key_and_delete, NULL);
    std::cout << std::endl;

    return 0;
}
