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

class Graph {
    private:
    std::vector<std::vector<std::pair<int, int> > > adj_table;
    int edge_counter;

    public:
    Graph(int n_points): adj_table(n_points, std::vector<std::pair<int, int> >()), edge_counter(0) {}
    void AddEdge(int v1, int v2) {
        adj_table.at(v1).push_back(std::make_pair(v2, edge_counter));
        adj_table.at(v2).push_back(std::make_pair(v1, edge_counter));
        edge_counter++;
    }
    const std::vector<std::pair<int, int> >& GetNeighbours(int node_number) const {
        return adj_table[node_number];
    }
    int GetSize() const {
        return adj_table.size();
    }
};

std::vector<int> find_bridges(const Graph& graph) {
    std::vector<int> bridges;
    std::vector<int> in_times(graph.GetSize(), -1);
    std::vector<std::pair<int, int> > node_stack;
    node_stack.push_back(std::make_pair(0, 0));
    in_times.at(0) = 0;
    int time = 0;
    int next_entry_point = 1;
    while (node_stack.size() > 0) {
        // Depth-traversing the graph while noting the node entry times
        time++;
        int current = node_stack.back().first;
        int checked = node_stack.back().second;
        node_stack.pop_back();
        bool has_unvisited_neighbours = false;
        const std::vector<std::pair<int, int>>& neighbours = graph.GetNeighbours(current);
        for (int i = checked; i < neighbours.size(); i++) {
            int to_put = neighbours.at(i).first;
            if (in_times.at(to_put) < 0) {
                node_stack.push_back(std::make_pair(current, checked + 1));
                node_stack.push_back(std::make_pair(to_put, 0));
                in_times.at(to_put) = time;
                has_unvisited_neighbours = true;
                break;
            }
        }
        if (has_unvisited_neighbours) continue;

        // When node has no unvisited neighbours left, we pull back the smallest entry
        // time from the neighbouring nodes (with the 'parent' edge excluded)
        int parent = node_stack.size() > 0 ? node_stack.back().first : -1;
        int parent_edge;
        bool checked_parent = false;
        for (int i = 0; i < neighbours.size(); i++) {
            int neighbour = neighbours.at(i).first;
            if (neighbour == parent && (!checked_parent)) {
                checked_parent = true;
                parent_edge = neighbours.at(i).second;
                continue;
            }
            in_times.at(current) = in_times.at(current) > in_times.at(neighbour)
                ? in_times.at(neighbour) : in_times.at(current);
        }
        // If the parent node's entry time is larger then the current node's entry time,
        // the parent edge cannot be a bridge. Otherwise it must be a bridge.
        if (parent >= 0) {
            if (in_times.at(parent) < in_times.at(current)) {
                bridges.push_back(parent_edge);
            }
        }

        // If the graph is not connected, we need to find a new entry point for the traversal.
        if (node_stack.size() == 0) {
            for (int i = next_entry_point; i < graph.GetSize(); i++) {
                if (in_times.at(i) < 0) {
                    next_entry_point = i + 1;
                    in_times.at(i) = time;
                    node_stack.push_back(std::make_pair(i, 0));
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
    Graph graph(n_nodes);
    for (int i = 0; i < n_edges; i++) {
        int node_1, node_2;
        input >> node_1 >> node_2;
        node_1--;
        node_2--;
        graph.AddEdge(node_1, node_2);
    }
    std::vector<int> bridges = find_bridges(graph);
    std::sort(bridges.begin(), bridges.end());
    output << bridges.size() << '\n';
    for (int i = 0; i < bridges.size(); i++)  {
        output << bridges.at(i) << " ";
    }
    output << '\n';

    return 0;
}
