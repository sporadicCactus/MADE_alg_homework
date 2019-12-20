#include <random>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <ctime>
#include <set>
#include <queue>

struct Point {
    float x;
    float y;
    static float Dist(const Point& p1, const Point &p2) {
        return sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2));
    }
    Point(float x_, float y_): x(x_), y(y_) {}
};

std::vector<Point> generate_points(int n_points) {
    std::vector<Point> points;
    srand(time(0));
    for (int i = 0; i < n_points; i++) {
        float r = (float)rand() / (float)RAND_MAX;
        float phi = (float)rand() / (float)RAND_MAX;
        Point p(std::cos(2*M_PI*phi)*sqrt(-2*log(r)),
            std::sin(2*M_PI*phi)*sqrt(-2*log(r)));
        points.push_back(p);
    }
    return points;
}

std::vector<std::vector<float> > generate_adj_matrix(std::vector<Point> points) {
    int n_points = points.size();
    std::vector<std::vector<float> > adj_matrix(n_points,
        std::vector<float>(n_points, 0));
    for (int i = 0; i < n_points; i++) {
        for (int j = i + 1; j < n_points; j++) {
            float dist = Point::Dist(points[i], points[j]);
            adj_matrix[i][j] = dist;
            adj_matrix[j][i] = dist;
        }
    }
    return adj_matrix;
}

float arc(int begin, int end, std::set<int>& inter,
    const std::vector<std::vector<float> >& adj_matrix) {
    if (inter.empty()) return adj_matrix[begin][end];
    std::set<float> results;
    int size = inter.size();
    for (int i = 0; i < size; i++) {
        std::set<int>::iterator it = inter.begin();
        for (int j = 0; j < i; j++) it++;
        int drop = *it;
        inter.erase(it);
        results.insert(adj_matrix[begin][*it] + arc(*it, end, inter, adj_matrix));
        inter.insert(drop);
    }
    return *results.begin();
}

float bruteforce_TSP(const std::vector<std::vector<float> >& adj_matrix) {
    int n_points = adj_matrix.size();
    std::set<int> inter;
    for (int i = 0; i < n_points; i++) {
        inter.insert(i);
    }
    inter.erase(inter.begin());
    float min_path_len = arc(0, 0, inter, adj_matrix);
    inter.insert(0);
    for (int i = 1; i < n_points; i++) {
        inter.erase(inter.find(i));
        float path_len = arc(i, i, inter, adj_matrix);
        inter.insert(i);
        min_path_len = path_len < min_path_len ? path_len : min_path_len;
    }
    return min_path_len;
}

struct node_with_neighbour {
    int node;
    int neighbour;
    float dist;
    bool operator<(const node_with_neighbour& other) const {
        return dist > other.dist;
    }
};

float min_tree_weight(const std::vector<std::vector<float> >& adj_matrix) {
    if (adj_matrix.size() < 2) return 0;
    float tree_weight = 0;
    std::vector<bool> tree_nodes(adj_matrix.size(), false);
    tree_nodes[0] = true;
    std::priority_queue<node_with_neighbour> front;
    for (int i = 0; i < adj_matrix.size(); i++) {
        node_with_neighbour front_node;
        front_node.node = i;
        front_node.neighbour = 0;
        front_node.dist = adj_matrix[0][i];
        front.push(front_node);
    }
    while (front.size() > 0) {
        node_with_neighbour considered = front.top();
        front.pop();
        if (tree_nodes[considered.node]) continue;
        tree_nodes[considered.node] = true;
        tree_weight += considered.dist;
        for (int i = 0; i < adj_matrix.size(); i++) {
            if (tree_nodes[i]) continue;
            node_with_neighbour new_front_node;
            new_front_node.node = i;
            new_front_node.neighbour = considered.node;
            new_front_node.dist = adj_matrix[considered.node][i];
            front.push(new_front_node);
        }
    }
    return tree_weight;
}

Point brute_vs_tree(std::vector<Point> points) {
    const std::vector<std::vector<float> > adj_matrix = generate_adj_matrix(points);
    float tree_estimate = 2*min_tree_weight(adj_matrix);
    float true_min = bruteforce_TSP(adj_matrix);
    return Point(true_min, tree_estimate);
}

void tabulate(int n_samples) {
    std::cout << "--------------------------------------------------" << '\n';
    std::cout << "        Tree estimation    |     True minimum     " << '\n';
    std::cout << "n_pts   average   std. dev |    average   std. dev" << '\n';
    std::cout << "--------------------------------------------------" << '\n';
    for (int n_points = 2; n_points <= 8; n_points++) {
        std::vector<Point> results;
        float av_est = 0;
        float av_tr = 0;
        float std_est = 0;
        float std_tr = 0;
        for (int i = 0; i < n_samples; i++) {
            std::vector<Point> points(generate_points(n_points));
            Point res = brute_vs_tree(points);
            av_est += res.y;
            av_tr += res.x;
            results.push_back(res);
        }
        av_est /= n_samples;
        av_tr /= n_samples;
        for (int i = 0; i < n_samples; i++) {
            std_est += pow(results[i].y - av_est, 2);
            std_tr += pow(results[i].x - av_tr, 2);
        }
        std_est = sqrt(std_est / n_samples);
        std_tr = sqrt(std_tr / n_samples);
        std::cout << std::fixed << std::setw(4) << n_points;
        std::cout << std::fixed << std::setw( 11 )
            << std::setprecision( 6 ) << av_est;
        std::cout << std::fixed << std::setw( 11 )
            << std::setprecision( 6 ) << std_est;
        std::cout << " |";
        std::cout << std::fixed << std::setw( 11 )
            << std::setprecision( 6 ) << av_tr;
        std::cout << std::fixed << std::setw( 11 )
            << std::setprecision( 6 ) << std_tr;
        std::cout << '\n';
    }
}


int main() {
    std::cout << "Number of samples: \n";
    int n_samples;
    std::cin >> n_samples;
    tabulate(n_samples);

    return 0;
}