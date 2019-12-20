// Дан неориентированный связный граф. Требуется
// найти вес минимального остовного дерева в этом графе. 
// Вариант 1. С помощью алгоритма Прима.
// Вариант 2. С помощью алгоритма Крускала.
// Вариант 3. С помощью алгоритма Борувки.
// Ваш номер варианта прописан в ведомости.
// Формат ввода
// Первая строка содержит два натуральных числа n и m — количество
// вершин и ребер графа соответственно (1 ≤ n ≤ 20000, 0 ≤ m ≤ 100000). 
// Следующие m строк содержат описание ребер по одному на строке. 
// Ребро номер i описывается тремя натуральными числами bi, ei и wi — номера
// концов ребра и его вес соответственно (1 ≤ bi, ei ≤ n, 0 ≤ wi ≤ 100000).

// Формат вывода
// Выведите единственное целое число - вес минимального остовного дерева.


#include <iostream>
#include <queue>
#include <set>
#include <vector>

struct arrow {
    int target_node;
    int weight;
    arrow(int target_, int w_): target_node(target_), weight(w_) {}
};

struct node_with_neighbour {
    int node;
    int neighbour;
    int dist;
    bool operator<(const node_with_neighbour& other) const {
        return dist > other.dist;
    }
};

int min_tree_weight(const std::vector<std::vector<arrow> >& adj_table) {
    if (adj_table.size() < 2) return 0;
    int tree_weight = 0;
    std::vector<bool> tree_nodes(adj_table.size(), false);
    tree_nodes[0] = true;
    std::priority_queue<node_with_neighbour> front;
    for (int i = 0; i < adj_table[0].size(); i++) {
        node_with_neighbour front_node;
        front_node.node = adj_table[0][i].target_node;
        front_node.neighbour = 0;
        front_node.dist = adj_table[0][i].weight;
        front.push(front_node);
    }
    while (front.size() > 0) {
        node_with_neighbour considered = front.top();
        front.pop();
        if (tree_nodes[considered.node]) continue;
        tree_nodes[considered.node] = true;
        tree_weight += considered.dist;
        for (int i = 0; i < adj_table[considered.node].size(); i++) {
            if (tree_nodes[adj_table[considered.node][i].target_node]) continue;
            node_with_neighbour new_front_node;
            new_front_node.node = adj_table[considered.node][i].target_node;
            new_front_node.neighbour = considered.node;
            new_front_node.dist = adj_table[considered.node][i].weight;
            front.push(new_front_node);
        }
    }
    return tree_weight;
}

int main() {
    int n_nodes;
    std::cin >> n_nodes;
    int n_edges;
    std::cin >> n_edges;
    std::vector<std::vector<arrow> > adj_table(n_nodes, std::vector<arrow>());
    for (int i = 0; i < n_edges; i++) {
        int node1;
        std::cin >> node1;
        node1--;
        int node2;
        std::cin >> node2;
        node2--;
        int weight;
        std::cin >> weight;
        adj_table[node1].push_back(arrow(node2, weight));
        adj_table[node2].push_back(arrow(node1, weight));
    }
    std::cout << min_tree_weight(adj_table) << '\n';
    return 0;
}
