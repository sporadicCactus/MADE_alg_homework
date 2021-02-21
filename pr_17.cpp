#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <set>
#include <unordered_set>
#include <queue>
#include <algorithm>

struct Node {
    Node *fallback;
    Node *term_fb;
    int terminates;
    std::unordered_map<char, Node*> children;
    Node(): fallback(nullptr), term_fb(nullptr), terminates(-1) {}
    Node(Node* root): fallback(root), term_fb(nullptr), terminates(-1) {}
    Node* child(char chr) {
        std::unordered_map<char, Node*>::iterator it = children.find(chr);
        if (it != children.end()) {
            return it->second;
        } else {
            return nullptr;
        }
    }
};

class Trie {
    private:
    bool empty;
    Node *root;
    Node *current;
    Node *terminating_node;
    std::vector<int> pattern_sizes;
    Node* next_node(Node *current_node, char next_char) {
        Node* next = current_node->child(next_char);
        if (next) return next;
        if (!current_node->fallback) {
            return root;
        }
        next = next_node(current_node->fallback, next_char);
        current_node->children.insert(std::make_pair(next_char, next));
        return next;
    }
    std::pair<int, Node*> backtrack_termination(Node *current_node) {
        if (current_node->term_fb)
            return std::make_pair(current_node->terminates, current_node->term_fb); 
        if (!current_node->fallback) {
            current_node->term_fb = current_node;
            return std::make_pair(-1, current_node);
        }
        std::pair<int, Node*> prev = backtrack_termination(current_node->fallback);
        if (prev.first >= 0) {
            current_node->term_fb = current_node->fallback;
            return std::make_pair(current_node->terminates, current_node->term_fb);
        }
        current_node->term_fb = prev.second;
        return  std::make_pair(current_node->terminates, current_node->term_fb);
    }
    Node* build_trie(std::vector<std::string> patterns) {
        Node *root = new Node;
        int char_pos = 0;
        std::unordered_map<int, Node*> insertion_front;
        for (int i = 0; i < patterns.size(); i++) {
            insertion_front.insert(std::make_pair(i, root));
        }
        while (insertion_front.size() > 0) {
            std::vector<int> ended_patterns;
            for (std::unordered_map<int, Node*>::iterator num_and_node
                = insertion_front.begin(); num_and_node != insertion_front.end();
                num_and_node++) {
                bool last_char = (char_pos == patterns[num_and_node->first].size() - 1);
                char chr = patterns[num_and_node->first][char_pos];
                Node *current = num_and_node->second;
                Node *new_node = current->child(chr);
                if (new_node) {
                    if (last_char) {
                        new_node->terminates = num_and_node->first;
                    }
                    num_and_node->second = new_node;
                } else {
                    Node *new_node = new Node(root);
                    Node *fb = current->fallback;
                    while (fb) {
                        if (fb->child(chr)) {
                            new_node->fallback = fb->child(chr);
                            break;
                        }
                        fb = fb->fallback;
                    }
                    if (last_char) {
                        new_node->terminates = num_and_node->first;
                    }
                    current->children.insert(std::make_pair(chr, new_node));
                    num_and_node->second = new_node;
                }
                if (last_char) {
                    ended_patterns.push_back(num_and_node->first);
                }
            }
            for (int i = 0; i < ended_patterns.size(); i++) {
                insertion_front.erase(ended_patterns[i]);
            }
            char_pos++;
        }
        return root;
    }

    public:
    Trie(): empty(true) {}
    Trie(const std::vector<std::string>& patterns_) {
        for (int i = 0; i < patterns_.size(); i++) {
            pattern_sizes.push_back(patterns_[i].size());
        }
        root = build_trie(patterns_);
        current = root;
        empty = false;
    }
    ~Trie() {
        if (empty) return;
        std::queue<Node*> node_queue;
        std::set<Node*> deleted_nodes;
        node_queue.push(root);
        while (node_queue.size() > 0) {
            Node *current = node_queue.front();
            node_queue.pop();
            if (deleted_nodes.find(current) != deleted_nodes.end()) continue;
            for (std::pair<char, Node*> char_and_node : current->children) {
                node_queue.push(char_and_node.second);
            }
            deleted_nodes.insert(current);
            delete current;
        }
    }
    bool Fill(const std::vector<std::string>& patterns_) {
        if (!empty) return false;
        for (int i = 0; i < patterns_.size(); i++) {
            pattern_sizes.push_back(patterns_[i].size());
        }
        root = build_trie(patterns_);
        current = root;
        empty = false;
        return true; 
    }
    bool Fill(std::vector<std::string>&& patterns_) {
        if (!empty) return false;
        for (int i = 0; i < patterns_.size(); i++) {
            pattern_sizes.push_back(patterns_[i].size());
        }
        root = build_trie(patterns_);
        current = root;
        empty = false;
        return true;
    }
    void Consume(char next_char) {
        current = next_node(current, next_char);
        terminating_node = current;
    }
    int GetTerminated() {
        if (terminating_node == root) return -1;
        std::pair<int, Node*> term_and_fb = backtrack_termination(terminating_node);
        if (term_and_fb.first < 0 && terminating_node == current) {
            term_and_fb = backtrack_termination(term_and_fb.second);
        }
        terminating_node = term_and_fb.second;
        return term_and_fb.first;
    }
    void Print() const {
        std::queue<Node*> node_queue;
        std::unordered_map<Node*, int> node_numbers;
        int counter = 0;
        node_queue.push(root);
        node_numbers.insert(std::make_pair(current, counter));
        counter++;
        while (node_queue.size() > 0) {
            Node* current = node_queue.front();
            node_queue.pop();
            int current_number = node_numbers.find(current)->second;
            std::cout << "Number: " << current_number << '\n';
            for (std::pair<char, Node*> char_and_node : current->children) {
                std::cout << '\'' << char_and_node.first << '\'' << ": " << counter << "; ";
                node_queue.push(char_and_node.second);
                node_numbers.insert(std::make_pair(char_and_node.second, counter));
                counter++;
            }
            std::cout << '\n';
            if (current->fallback) {
                std::cout << "Fallback: " << node_numbers.find(current->fallback)->second << '\n';
            }
            std::cout << '\n';
        }
    }
};

class Matcher {
    private:
    int pattern_length;
    int left_padding;
    int right_padding;
    int n_subpatterns;
    std::vector<std::string> patterns;
    std::vector<int> displacements;
    std::vector<std::vector<int> > pattern_places;
    Trie trie;

    public:
    Matcher(const std::string& pattern) {
        pattern_length = pattern.size();
        left_padding = 0;
        for (int i = 0; i < pattern.size(); i++) {
            if (pattern[i] != '?') break;
            left_padding++; 
        }
        if (left_padding == pattern.size()) return;
        right_padding = 0;
        for (int i = pattern.size() - 1; i >= 0; i--) {
            if (pattern[i] != '?') break;
            right_padding++;
        }
        std::unordered_map<std::string, int> known_patterns;
        std::string accumulator = "";
        n_subpatterns = 0;
        int last_pattern_end = 0;
        for (int i = left_padding; i < pattern.size() - right_padding; i++) {
            char chr = pattern[i];
            if (chr != '?') {
                if (accumulator.size() == 0) {
                    n_subpatterns++;
                }
                accumulator.push_back(chr);
                continue;
            }
            if (accumulator.size() > 0) {
                std::unordered_map<std::string, int>::iterator it = 
                    known_patterns.find(accumulator); 
                if (it == known_patterns.end()) {
                    known_patterns.insert(std::make_pair(accumulator, known_patterns.size()));
                    patterns.push_back(std::move(accumulator));
                    pattern_places.push_back(std::vector<int>(1, n_subpatterns - 1));
                } else {
                    pattern_places[it->second].push_back(n_subpatterns - 1);
                }
                displacements.push_back(i - last_pattern_end - 1);
                last_pattern_end = i - 1;
                accumulator.clear();
            }
        }
        std::unordered_map<std::string, int>::iterator it = 
            known_patterns.find(accumulator); 
        if (it == known_patterns.end()) {
            known_patterns.insert(std::make_pair(accumulator, known_patterns.size()));
            patterns.push_back(std::move(accumulator));
            pattern_places.push_back(std::vector<int>(1, n_subpatterns - 1));
        } else {
            pattern_places[it->second].push_back(n_subpatterns - 1);
        }
        displacements.push_back(pattern.size() - right_padding - 1 - last_pattern_end);
        trie.Fill(patterns);
    }
    void ParseStream(std::istream& in, std::ostream& out) {
        std::vector<std::deque<int> > checker(n_subpatterns, std::deque<int>());
        char c;
        int char_counter = 0;
        std::deque<int> almost_complete;
        int cutoff = left_padding + patterns[0].size() - 1;
        while (in >> c) {
            if (c == '\n') break;
            trie.Consume(c);
            int pt_num = 0;
            while (true) {
                pt_num = trie.GetTerminated();
                if (pt_num < 0) break;
                if (char_counter < cutoff) break;
                for (int j = 0; j < pattern_places[pt_num].size(); j++) {
                    int place = pattern_places[pt_num][j];
                    if (n_subpatterns == 1) {
                        almost_complete.push_back(char_counter + right_padding);
                        continue;
                    }
                    std::deque<int>& active_checker = checker[place];
                    while (active_checker.size() > 0) {
                        if (active_checker.front() < char_counter) {
                            active_checker.pop_front();
                        } else {
                            break;
                        }
                    }
                    if (place == n_subpatterns - 1) {
                        if (active_checker.size() == 0) continue;
                        if (active_checker.front() == char_counter) {
                            almost_complete.push_back(char_counter + right_padding);
                        }
                        continue;
                    }
                    if (place > 0) {
                        if (active_checker.size() == 0) continue;
                        if (active_checker.front() != char_counter) continue;
                    }
                    int expected = char_counter + displacements[place + 1];
                    checker[place + 1].push_back(expected);
                }
            }
            while (almost_complete.size() > 0) {
                if (almost_complete.front() > char_counter) break;
                out << char_counter - pattern_length + 1 << ' ';
                almost_complete.pop_front();
            }
            char_counter++;
        }
    }
};

int main() {
    std::ifstream input("input.txt");
    std::ofstream output("output.txt");
    std::string pattern;
    input >> pattern;
    Matcher matcher(pattern);
    matcher.ParseStream(input, output);
    output << '\n';
    return 0;
}
