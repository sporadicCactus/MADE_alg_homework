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
#include <stack>
#include <algorithm>

class Graph {
    private:
    std::vector<std::vector<std::pair<int, int> > > adj_table;
    int edge_counter;

    public:
    Graph(int n_points): adj_table(n_points, std::vector<std::pair<int, int> >()), edge_counter(0) {}
    void AddEdge(int v1, int v2) {
        edge_counter++;
        adj_table.at(v1).push_back(std::make_pair(v2, edge_counter));
        adj_table.at(v2).push_back(std::make_pair(v1, edge_counter));
    }
    const std::vector<std::pair<int, int> >& GetNeighbours(int node_number) const {
        return adj_table[node_number];
    }
    int GetSize() const {
        return adj_table.size();
    }
};


class BridgeFinder {
    private:
    const Graph& graph;
    std::vector<int> in_times;
    std::vector<int> bridges;
    int next_entry_point;

    // The first int in the pair is a node number, the other one is
    // the number of already checked neighbours of this node
    std::stack<std::pair<int, int> > stack;

    int find_next_node() {
        int current_node = stack.top().first;
        int n_neighbours_checked = stack.top().second;
        const std::vector<std::pair<int, int> >& neighbours = graph.GetNeighbours(current_node);
        for (int i = n_neighbours_checked; i < neighbours.size(); i++) {
            int next_candidate = neighbours[i].first;
            // Checking if the candidate node is yet unvisited
            if (in_times[next_candidate] < 0) {
                stack.pop();
                stack.push(std::make_pair(current_node, i + 1));
                return next_candidate;
            }
        }
        return -1;
    }

    int pull_back_entry_time_and_get_parent_edge(int current_node) {
        int parent_node = stack.size() > 0 ? stack.top().first : -1;
        int parent_edge = -1;
        bool checked_parent = false;
        const std::vector<std::pair<int, int> >& neighbours = graph.GetNeighbours(current_node);
        for (int i = 0; i < neighbours.size(); i++) {
            int neighbour = neighbours[i].first;
            if ((neighbour == parent_node) && (!checked_parent)) {
                checked_parent = true;
                parent_edge = neighbours[i].second;
                continue;
            }
            in_times[current_node] = in_times[current_node] > in_times[neighbour]
                ? in_times[neighbour] : in_times[current_node];
        }
        return parent_edge;
    }

    public:
    // Entry time for each node is initialized to -1
    BridgeFinder(const Graph& graph_): graph(graph_), in_times(graph_.GetSize(), -1) {
        if (graph.GetSize() < 1) return;
        // Initialize stack with some node
        stack.push(std::make_pair(0, 0));
        // The entry time for the initial node is zero
        in_times[0] = 0;
        // When the initial node's connected component is
        // fully explored, this will be the starting point
        // for searching another connected component.
        next_entry_point = 1;
        
        // Setting the time counter
        int time = 0;
        // Repeating while we have nodes to consider
        while (stack.size() > 0) { 
            time++;
            int next_node = find_next_node();
            // Check if there is a next node to visit
            if (next_node >= 0) {
                stack.push(std::make_pair(next_node, 0));
                in_times[next_node] = time;
                continue;
            }
            
            // If there is no next node to visit, we start to retract
            // First, we update the entry time of the node on the top of the stack,
            // making it the smallest entry time of its' neighbours (but we don't
            // follow the edge we used to get to this node)
            int current_node = stack.top().first;
            stack.pop();
            int parent_edge = pull_back_entry_time_and_get_parent_edge(current_node);

            // Now if the current node's entry time is smaller than its' parent's
            // entry time, there is a "shortcut" to the visited part of the graph,
            // so the parent edge cannot be a bridge. Otherwise it must be a bridge
            if (stack.size() > 0) {
                int parent_node = stack.top().first;
                if (in_times[current_node] > in_times[parent_node]) {
                    bridges.push_back(parent_edge);
                }
            }

            // It could be the stack is empty now, but the graph is not fully explored:
            // we couldn't reach disconnected components. So we need to check if there is
            // some yet unvisited node in the graph, and use it as a new entry point
            if (stack.size() == 0) {
                for (int i = next_entry_point; i < graph.GetSize(); i++) {
                    if (in_times.at(i) < 0) {
                        next_entry_point = i + 1;
                        in_times.at(i) = time;
                        stack.push(std::make_pair(i, 0));
                        break;
                    }
                }   
            }
        }
    }
    const std::vector<int>& GetBridges() {
        return bridges;
    }
};


std::vector<int> find_bridges(const Graph& graph) {
    BridgeFinder bridge_finder(graph);
    return bridge_finder.GetBridges();
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
