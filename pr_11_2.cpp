// Дан невзвешенный неориентированный граф. В графе может быть
// несколько кратчайших путей между какими-то вершинами.
// Найдите количество различных кратчайших путей между заданными
// вершинами. Требуемая сложность O(V+E).
// Ввод: v: кол-во вершин (макс. 50000), n: кол-во ребер (макс. 200000),
// n пар реберных вершин, пара вершин v, w для запроса.
// Вывод: количество кратчайших путей от v к w.


#include <iostream>
#include <vector>
#include <set>

struct node_with_count {
    int node;
    int count;
};

int count_shortest_paths(const std::vector<std::vector<int> > adj_table,
    int node_start, int node_finish) {
    std::vector<int> n_shortest_paths(adj_table.size(), 0);
    n_shortest_paths.at(node_start) = 1;
    std::set<int> front_nodes;
    front_nodes.insert(node_start);
    std::vector<node_with_count> accumulator(0);
    while ((n_shortest_paths.at(node_finish) == 0)
        && (front_nodes.size() > 0)) {
        for (std::set<int>::iterator it = front_nodes.begin();
            it != front_nodes.end(); it++) {
            int current = *it;
            for (int i = 0; i < adj_table.at(current).size(); i++) {
                int next = adj_table.at(current).at(i);
                if (n_shortest_paths.at(next) == 0) {
                    node_with_count next_with_count;
                    next_with_count.node = next;
                    next_with_count.count = n_shortest_paths.at(current);
                    accumulator.push_back(next_with_count);
                }
            }
        }
        front_nodes.clear();
        while (accumulator.size() > 0) {
            node_with_count current = accumulator.back();
            accumulator.pop_back();
            front_nodes.insert(current.node);
            n_shortest_paths.at(current.node) += current.count;
        }
    }
    return n_shortest_paths.at(node_finish);
}

int main() {
    int n_nodes;
    std::cin >> n_nodes;
    int n_edges;
    std::cin >> n_edges;
    std::vector<std::vector<int> > adj_table(n_nodes, std::vector<int>());
    for (int i = 0; i < n_edges; i++) {
        int node_1, node_2;
        std::cin >> node_1 >> node_2;
        adj_table.at(node_1).push_back(node_2);
        adj_table.at(node_2).push_back(node_1);
    }

    int node_start, node_finish;
    std::cin >> node_start >> node_finish;

    std::cout << count_shortest_paths(adj_table, node_start, node_finish);
    std::cout << '\n';

    return 0;
}
