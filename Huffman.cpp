#include <vector>
#include <queue>
#include <deque>
#include <algorithm>
#include "Huffman.h"

typedef unsigned char byte;

class BitsWriter {
    private:
    byte accumulator;
    char n_bits;

    public:
    BitsWriter() : accumulator(0), n_bits(0) {}
    void WriteBit(IOutputStream &out, bool bit);
    void WriteByte(IOutputStream &out, byte value);
    void Flush(IOutputStream &out);
};

void  BitsWriter::WriteBit(IOutputStream &out, bool bit) {
    n_bits++;
    accumulator = accumulator << 1;
    accumulator |= (byte)bit;

    if (n_bits == 8) {
        out.Write(accumulator);
        accumulator = 0;
        n_bits = 0;
    }
}

void BitsWriter::WriteByte(IOutputStream &out, byte value) {
    if (n_bits == 0) {
        out.Write(value);
        return;
    }
    accumulator = accumulator << (8 - n_bits) | value >> (8 - n_bits);
    out.Write(accumulator);
    accumulator = value;
}

void BitsWriter::Flush(IOutputStream &out) {
    if (n_bits == 0) {
        out.Write(accumulator);
        return;
    }
    accumulator = accumulator << (8 - n_bits);
    out.Write(accumulator);
    accumulator = 8  - n_bits;
    out.Write(accumulator);
    accumulator = 0;
    n_bits = 0;
}


class BitsDecoder {
    private:
    int payload;
    byte* buffer;

    public:
    BitsDecoder(byte *decoded_tree) : payload(0), buffer(decoded_tree) {}
    bool EatBit(bool bit) {
        byte *ptr = buffer + 3*payload;
        payload = *(ptr + 1 + bit);
        return bit ? (*ptr) & 1 : (*ptr) >> 1;
    }
    byte GetChar() {
        byte payload_ = (byte)payload;
        payload = 0;
        return payload_;
    }
};




struct Node {
    int weight;
    byte value;
    Node* parent;
    Node* left_child;
    Node* right_child;
    bool is_leaf;
    bool operator<(const Node &rhs) const {
        return this->weight > rhs.weight;
    }
    Node(int wt, byte vl, Node* pt, Node* lc, Node* rc, bool il) :
        weight(wt), value(vl), parent(pt), left_child(lc), right_child(rc),
        is_leaf(il) {}
};

Node* pop_from_heap(std::priority_queue<Node> &nodes_heap) {
    Node *node = new Node(nodes_heap.top());
    Node *lc = node->left_child;
    Node *rc = node->right_child;
    if (lc) lc->parent = node;
    if (rc) rc->parent = node;
    nodes_heap.pop();
    return node;
}

Node* build_tree(int *counter, Node** leaves_table) {
    std::priority_queue<Node> nodes_heap;
    for (int i = 0; i < 256; i++) {
        if (counter[i] > 0) {
            nodes_heap.push(
                Node(counter[i], (byte)i, NULL, NULL, NULL, true));
        }
    }
    while (nodes_heap.size() > 1) {
        Node *node1 = pop_from_heap(nodes_heap);
        Node *node2 = pop_from_heap(nodes_heap);
        if (!(node1->left_child) && !(node1->right_child))
            leaves_table[node1->value] = node1;
        if (!(node2->left_child) && !(node2->right_child))
            leaves_table[node2->value] = node2;
        nodes_heap.push(Node(
            node1->weight + node2->weight, 0, NULL, node1, node2, false));
    }
    Node *root = pop_from_heap(nodes_heap);
    return root;
}

byte* encode_tree_and_free(Node* tree_root, int n_leaves) {
    byte *buffer = (byte*)malloc(3*(n_leaves - 1));
    std::deque<Node*> stack;
    stack.push_back(tree_root);
    int h_node_counter = 0;
    int t_node_counter = 1;
    while (stack.size() > 0) {
        Node *current = stack.front();
        stack.pop_front();
        if (current->is_leaf) {
            delete current;
            continue;
        }
        byte *ptr = buffer + 3*h_node_counter;
        *ptr = 0;
        Node *lc = current->left_child;
        Node *rc = current->right_child;
        if (lc) {
            stack.push_back(lc);
            if (lc->is_leaf) {
                *(ptr + 1) = lc->value;
                *ptr += 2;
            } else {
                *(ptr + 1) = t_node_counter;
                t_node_counter++;
            }
        }
        if (rc) {
            stack.push_back(rc);
            if (rc->is_leaf) {
                *(ptr + 2) = rc->value;
                *ptr += 1;
            } else {
                *(ptr + 2) = t_node_counter;
                t_node_counter++;
            }
        }
        h_node_counter++;
        delete current;
    }
    return buffer;
}

void Encode(IInputStream &original, IOutputStream &compressed) {
    std::vector<byte> raw_bytes;
    int *counter = (int*)calloc(256, sizeof(int));

    byte value;
    while (original.Read(value)) {
        raw_bytes.push_back(value);
        counter[value]++;
    }

    Node **leaves_table = (Node**)calloc(256, sizeof(void*));
    Node *tree_root = build_tree(counter, leaves_table);
    free(counter);

    int n_leaves = 0;
    std::vector<std::vector<bool> > coding_table;
    for (int i = 0; i < 256; i++) {
        std::vector<bool> curr_vec;
        curr_vec.empty();
        Node *curr_node = leaves_table[i];
        if (!curr_node) {
            coding_table.push_back(curr_vec);
            continue;
        }
        n_leaves++;
        while (curr_node->parent) {
            if ((curr_node->parent)->left_child == curr_node) {
                curr_vec.push_back(false);
            } else {
                curr_vec.push_back(true);
            }
            curr_node = curr_node->parent;
        }
        std::reverse(curr_vec.begin(), curr_vec.end());
        coding_table.push_back(curr_vec);
    }
    free(leaves_table);

    byte *encoded_tree = encode_tree_and_free(tree_root, n_leaves);

    BitsWriter bits_writer;
    value = (byte)(n_leaves - 1);
    bits_writer.WriteByte(compressed, value);
    for (int i = 0; i < 3*(n_leaves - 1); i++)
        bits_writer.WriteByte(compressed, encoded_tree[i]);
    free(encoded_tree);
    for (int i = 0; i < raw_bytes.size(); i++) {
        byte value = raw_bytes.at(i);
        std::vector<bool> *code = &coding_table[value];
        for (int j = 0; j < code->size(); j++)
            bits_writer.WriteBit(compressed, code->at(j));
    }
    bits_writer.Flush(compressed);
}

void Decode(IInputStream &compressed, IOutputStream &original) {
    byte n_nodes;
    compressed.Read(n_nodes);
    byte *encoded_tree = (byte*)malloc(3*n_nodes);
    for (int i = 0; i < 3*n_nodes; i++) {
        byte value;
        compressed.Read(value);
        encoded_tree[i] = value;
    }
    BitsDecoder bits_decoder(encoded_tree);

    byte acc1;
    compressed.Read(acc1);
    byte acc2;
    compressed.Read(acc2);
    byte acc3;
    while (compressed.Read(acc3)) {
        for (int i = 0; i < 8; i++) {
            if (bits_decoder.EatBit(acc1 & 1 << (7-i))) {
                byte value = bits_decoder.GetChar();
                original.Write(value);
            }
        }
        acc1 = acc2;
        acc2 = acc3;
    }
    for (int i = 0; i < 8 - acc2; i++) {
        if (bits_decoder.EatBit(acc1 & 1 << (7-i))) {
            byte value = bits_decoder.GetChar();
            original.Write(value);
        }
    }
    free(encoded_tree);
}
