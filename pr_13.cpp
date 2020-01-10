// Написать алгоритм для решения игры в “пятнашки”. Решением
// задачи является приведение к виду:
// [ 1 2 3 4 ] [ 5 6 7 8 ] [ 9 10 11 12] [ 13 14 15 0 ],
// где 0 задает пустую ячейку. Достаточно найти хотя бы какое-то решение.
// Число перемещений костяшек не обязано быть минимальным.

// Формат ввода
// Начальная расстановка.

// Формат вывода
// Если вам удалось найти решение, то в первой строке файла выведите число
// перемещений, которое требуется сделать в вашем решении. А во второй
// строке выведите соответствующую последовательность ходов: L означает,
// что в результате перемещения костяшка сдвинулась влево, R – вправо,
// U – вверх, D – вниз. Если же выигрышная конфигурация недостижима,
// то выведите в выходной файл одно число −1.



#include <iostream>
#include <set>
#include <queue>
#include <deque>
#include <cstring>

typedef unsigned char uc;
typedef uint64_t st;

struct Node {
    st state;
    uc empty_row;
    uc empty_col;
    int heuristic;
    mutable int dist;
    mutable char prev_move;
    bool operator<(const Node& other) const {
        return state < other.state;
    }
    static int Heuristic(const int mat[4][4]) {
        int lin_conflicts = 0;
        int manhattan = 0;
        int temp_mat[4][4];
        std::memcpy(temp_mat, mat, 16*sizeof(int));
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                int temp = (mat[i][j] + 15) % 16;
                manhattan += abs(i - (temp / 4));
                manhattan += abs(j - (temp % 4));
                if ((i - temp / 4 != 0) | (temp == 15)) {
                    temp_mat[i][j] = 4;
                } else {
                    temp_mat[i][j] = temp % 4;
                }
            }
            for (int j = 0; j < 4; j++) {
                for (int k = j + 1; k < 4; k++) {
                    if ((temp_mat[i][j] < 4)
                        & (temp_mat[i][j] >= temp_mat[i][k]))
                        lin_conflicts++;
                }
            }
        }
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                int temp = (mat[j][i] + 15) % 16;
                if ((i - temp % 4 != 0) | (temp == 15)) {
                    temp_mat[j][i] = 4;
                } else {
                    temp_mat[j][i] = temp / 4;
                }
            }
            for (int j = 0; j < 4; j++) {
                for (int k = j + 1; k < 4; k++) {
                    if ((temp_mat[j][i] < 4)
                        & (temp_mat[j][i] >= temp_mat[k][i]))
                        lin_conflicts++;
                }
            }
        }
        return manhattan + 2*lin_conflicts;
    }
    static int Heuristic(st state_) {
        int mat[4][4];
        for (int i = 3; i >= 0; i--) {
            for (int j = 3; j >= 0; j--) {
                mat[i][j] = (state_ % 16);
                state_ = state_ >> 4;
            }
        }
        return Heuristic(mat);
    }
    Node SpawnChild(uc move) const {
        Node new_node;
        new_node.empty_row = move % 2 == 0 ? empty_row + 1 - move : empty_row;
        new_node.empty_col = move % 2 == 1 ? empty_col - 2 + move : empty_col;
        if ((new_node.empty_row > 3) | (new_node.empty_row < 0) |
            (new_node.empty_col > 3) | (new_node.empty_col < 0)) {
                new_node.heuristic = -1;
                return new_node;
        }
        int mat[4][4];
        st temp = state;
        for (int i = 3; i >= 0; i--) {
            for (int j = 3; j >= 0; j--) {
                mat[i][j] = temp % 16;
                temp = temp >> 4;
            }
        }
        mat[empty_row][empty_col] = mat[new_node.empty_row][new_node.empty_col];
        mat[new_node.empty_row][new_node.empty_col] = 0;
        new_node.dist = dist + 1;
        new_node.prev_move = move;
        new_node.heuristic = Heuristic(mat);
        new_node.state = 0;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                new_node.state = new_node.state << 4;
                new_node.state = new_node.state | mat[i][j];
            }
        }
        return new_node;
    }
};

struct weight_and_node {
    int weight;
    Node node;
    weight_and_node(int weight_, Node node_): weight(weight_), node(node_) {}
    bool operator<(const weight_and_node& other) const {
        return weight > other.weight;
    }
};

struct Visited {
    std::set<Node> pool;
    bool Insert(Node new_node) {
        std::set<Node>::iterator it = pool.find(new_node);
        if (it == pool.end()) {
            pool.insert(new_node);
            return true;
        } else {
            if (new_node.dist < it->dist) {
                it->dist = new_node.dist;
                it->prev_move = new_node.prev_move;
            }
            return false;
        }
    }
};

struct Front {
    std::priority_queue<weight_and_node> pool;

    Node Pop() {
        Node first = (pool.top()).node;
        pool.pop();
        return first;
    }

    void Insert(Node new_node, int weight) {
        pool.push(weight_and_node(weight, new_node));
    }
};

std::deque<uc> solve_fifteen(st start) {
    Node start_node;
    start_node.state = start;
    start_node.prev_move = 4;
    start_node.dist = 0;
    start_node.heuristic = Node::Heuristic(start);
    for (int i = 3; i >= 0; i--) {
        for (int j = 3; j >= 0; j--) {
            int temp = start % 16;
            if (temp == 0)  {
                start_node.empty_row = i;
                start_node.empty_col = j;
            }
            start = start >> 4;
        }
    }
    Visited visited_pool;
    Front front_pool;
    front_pool.Insert(start_node, start_node.heuristic);
    Node last_node;
    bool found_finish = false;
    if (start_node.heuristic == 0) {
        last_node = start_node;
        found_finish = true;
    }
    while (!found_finish) {
        Node considered = front_pool.Pop();
        if (!visited_pool.Insert(considered)) continue;
        for (int i = 0; i < 4; i++) {
            if ( (i + 2) % 4 == considered.prev_move) continue;
            Node new_node = considered.SpawnChild(i);
            if (new_node.heuristic < 0) continue;
            if (new_node.heuristic == 0) {
                last_node = new_node;
                found_finish = true;
                break;
            }
            front_pool.Insert(new_node, 2*new_node.heuristic + new_node.dist);
        }
    }
    std::deque<uc> moves;
    while (last_node.prev_move < 4) {
        uc move = last_node.prev_move;
        moves.push_front(move);
        last_node = last_node.SpawnChild((move + 2) % 4);
        last_node = *visited_pool.pool.find(last_node);
    }
    return moves;
}

bool is_valid_and_solvable(st start) {
    std::set<int> knuckles_set;
    std::deque<int> knuckles_vec;
    int empty;
    for (int i = 15; i >= 0; i--) {
        int current = (int)(start & 15);
        knuckles_set.insert(current);
        if (current == 0) {
            current += 16;
            empty = i;
        }
        knuckles_vec.push_front(current);
        start = start >> 4;
    }
    if ((knuckles_set.size() != 16)
        | (knuckles_set.find(0) == knuckles_set.end())) {
        return false;
    }
    int inverse_pairs = 0;
    for (int i = 0; i < 16; i++) {
        for (int j = i; j < 16; j++) {
            if (knuckles_vec[i] > knuckles_vec[j]) inverse_pairs++;
        }
    }
    return !(bool)((inverse_pairs + empty % 4 + empty / 4) % 2);
}

int main() {
    // std::ifstream input("in.txt");
    st start = 0;
    for (int i = 0; i < 16; i++) {
        int current;
        std::cin >> current;
        // input >> current;
        start = start << 4;
        start = start | current;
    }
    if (!is_valid_and_solvable(start)) {
        std::cout << -1 << '\n';
        return 0;
    }
    std::deque<uc> moves = solve_fifteen(start);
    std::cout << moves.size() << '\n';
    for (int i = 0; i < moves.size(); i++) {
        uc curr_move = moves[i];
        switch (curr_move) {
            case 0:
                std::cout << 'U';
                break;
            case 1:
                std::cout << 'R';
                break;
            case 2:
                std::cout << 'D';
                break;
            case 3:
                std::cout << 'L';
                break;
        }
    }
    std::cout << '\n';
    return 0;
}
