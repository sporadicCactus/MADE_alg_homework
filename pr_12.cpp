// Ребро неориентированного графа называется мостом, если удаление
// этого ребра из графа увеличивает число компонент связности.
// Дан неориентированный граф, требуется найти в нем все мосты.

// Ввод: Первая строка входного файла содержит два целых
// числа n и m — количество вершин и ребер графа
// соответственно (1 ≤ n ≤ 20000, 1 ≤ m ≤ 200000).
// Следующие m строк содержат описание ребер по одному на строке.
// Ребро номер i описывается двумя натуральными
// числами bi, ei — номерами концов ребра (1 ≤ bi, ei ≤ n).
// Вывод: Первая строка выходного файла должна содержать одно
// натуральное число b — количество мостов в заданном графе.
// На следующей строке выведите b целых чисел — номера ребер,
// которые являются мостами, в возрастающем порядке. Ребра
// нумеруются с единицы в том порядке, в котором они заданы во входном файле.


#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>

struct node_and_edge {
    int node;
    int edge;
    node_and_edge(int node_, int edge_): node(node_), edge(edge_) {}
};

struct node_and_checked {
    int node;
    int checked;
    node_and_checked(int node_, int checked_): node(node_),
        checked(checked_) {}
};

std::vector<int> find_bridges(
    const std::vector<std::vector<node_and_edge> > adj_table) {
    std::vector<int> bridges;
    std::vector<int> in_times(adj_table.size(), -1);
    std::vector<node_and_checked> node_stack;
    node_stack.push_back(node_and_checked(0, 0));
    in_times.at(0) = 0;
    int time = 0;
    int next_entry_point = 1;
    while (node_stack.size() > 0) {
        time++;
        int current = node_stack.back().node;
        int checked = node_stack.back().checked;
        node_stack.pop_back();
        bool has_unvisited_neighbours = false;
        for (int i = checked; i < adj_table.at(current).size(); i++) {
            int to_put = adj_table.at(current).at(i).node;
            if (in_times.at(to_put) < 0) {
                node_stack.push_back(node_and_checked(current, checked + 1));
                node_stack.push_back(node_and_checked(to_put, 0));
                in_times.at(to_put) = time;
                has_unvisited_neighbours = true;
                break;
            }
        }
        if (has_unvisited_neighbours) continue;
        int parent = node_stack.size() > 0 ? node_stack.back().node : -1;
        int parent_edge;
        bool checked_parent = false;
        for (int i = 0; i < adj_table.at(current).size(); i++) {
            int neighbour = adj_table.at(current).at(i).node;
            if (neighbour == parent && (!checked_parent)) {
                checked_parent = true;
                parent_edge = adj_table.at(current).at(i).edge;
                continue;
            }
            in_times.at(current) = in_times.at(current) > in_times.at(neighbour)
                ? in_times.at(neighbour) : in_times.at(current);
        }
        if (parent >= 0) {
            if (in_times.at(parent) < in_times.at(current)) {
                bridges.push_back(parent_edge);
            }
        }
        if (node_stack.size() == 0) {
            for (int i = next_entry_point; i < adj_table.size(); i++) {
                if (in_times.at(i) < 0) {
                    next_entry_point = i + 1;
                    in_times.at(i) = time;
                    node_stack.push_back(node_and_checked(i, 0));
                    break;
                }
            }
        }
    }
    return bridges;
}

int main() {
    std::ifstream input("bridges.in");
    std::ofstream output("bridges.out");
    int n_nodes;
    input >> n_nodes;
    int n_edges;
    input >> n_edges;
    std::vector<std::vector<node_and_edge> > adj_table(n_nodes,
        std::vector<node_and_edge>());
    for (int i = 0; i < n_edges; i++) {
        int node_1, node_2;
        input >> node_1 >> node_2;
        adj_table.at(node_1 - 1).push_back(node_and_edge(node_2 - 1, i + 1));
        adj_table.at(node_2 - 1).push_back(node_and_edge(node_1 - 1, i + 1));
    }

    std::vector<int> bridges = find_bridges(adj_table);
    std::sort(bridges.begin(), bridges.end());
    output << bridges.size() << '\n';
    for (int i = 0; i < bridges.size(); i++)  {
        output << bridges.at(i) << " ";
    }
    output << '\n';

    return 0;
}
