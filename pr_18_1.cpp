#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <cmath>
#include <stack>

double EPSILON = 0.000000001;

struct Vector {
    double x;
    double y;
    Vector(double x_, double y_): x(x_), y(y_) {}
    double dot(Vector pt) const {
        return x*pt.x + y*pt.y;
    }
    double cross(Vector pt) const {
        return x*pt.y - y*pt.x;
    }
    double len() const {
        return std::sqrt(x*x + y*y);
    }
    Vector operator-(Vector other) const {
        return Vector(x - other.x, y - other.y);
    }
    Vector operator+(Vector other) const {
        return Vector(x + other.x, y + other.y);
    }
};

struct AngleAndVectorComparator {
    bool operator() (std::pair<double, Vector> x, std::pair<double, Vector> y) {
        return x.first < y.first;
    }
};

typedef std::priority_queue<std::pair<double, Vector>,
    std::vector<std::pair<double, Vector> >, AngleAndVectorComparator> PointsQueue;

void resolve_equal_angles(Vector start, PointsQueue& points_queue) {
    Vector top_point = points_queue.top().second;
    double top_angle = points_queue.top().first;
    points_queue.pop();
    while (points_queue.size() > 0) {
        if (points_queue.top().first == top_angle) {
            if ((points_queue.top().second - start).len() > (top_point - start).len()) {
                top_point = points_queue.top().second;
            }
            points_queue.pop();
        } else {
            break;
        }
    }
    points_queue.push(std::make_pair(top_angle, top_point));
}

void add_to_boundary(Vector new_point, std::vector<Vector>& boundary) {
    if (boundary.size() < 2) {
        boundary.push_back(new_point);
        return;
    }
    Vector prev_edge = boundary.back() - boundary[boundary.size() - 2];
    Vector new_edge = new_point - boundary.back();
    bool left_turn = (prev_edge.cross(new_edge) > 0.);
    while (!left_turn) {
        boundary.pop_back();
        if (boundary.size() < 2) break;
        new_edge = new_point - boundary.back();
        prev_edge = boundary.back() - boundary[boundary.size() - 2];
        left_turn = (prev_edge.cross(new_edge) > 0.);
    }
    boundary.push_back(new_point);
}

std::vector<Vector> build_boundary(int start_ind, const std::vector<Vector>& points, double regularizer) {
    Vector start = points[start_ind] - Vector(regularizer, 0.);
    PointsQueue points_queue;
    for (int i = 0; i < points.size(); i++) {
        if (i == start_ind) continue;
        double sin = (points[i] - start).cross(Vector(1., 0.)) / (points[i] - start).len();
        points_queue.push(std::make_pair(sin, points[i]));
    }
    std::vector<Vector> boundary;
    boundary.push_back(start);
    while (points_queue.size() > 0) {
        Vector new_point = points_queue.top().second;
        points_queue.pop();
        add_to_boundary(new_point, boundary);
    }
    start = start + Vector(regularizer, 0);
    boundary[0] = start;
    boundary.push_back(start);
    return boundary;
}

double compute_perimeter(const std::vector<Vector>& boundary) {
    double accumulator = 0;
    for (int i = 1; i < boundary.size(); i++) {
        accumulator += (boundary[i] - boundary[i - 1]).len();
    }
    return accumulator;
}

std::pair<int, double> find_starting_point_and_regularizer(const std::vector<Vector>& points) {
    Vector start = points[0];
    int start_ind = 0;
    double regularizer = 0;
    for (int i = 1; i < points.size(); i++) {
        regularizer += std::abs(points[i].x) + std::abs(points[i].y);
        if (points[i].x > start.x) continue;
        if (points[i].x < start.x) {
            start = points[i];
            start_ind = i;
            continue;
        }
        if (points[i].y < start.y) {
            start = points[i];
            start_ind = i;
        }
    }
    regularizer /= points.size();
    regularizer *= EPSILON;
    return std::make_pair(start_ind, regularizer);
}

// A point is represented as a vector from the coordinate origin to
// the respective point
double hull_perimeter(const std::vector<Vector>& points) {
    if (points.size() < 2) return 0.;
    if (points.size() == 2) {
        return 2*(points[0] - points[1]).len();
    }
    int start_ind;
    double regularizer;
    std::tie(start_ind, regularizer) = find_starting_point_and_regularizer(points);
    std::vector<Vector> boundary = build_boundary(start_ind, points, regularizer);
    return compute_perimeter(boundary);
}

int main() {
    int n_points;
    std::cin >> n_points;
    // A point is represented as a vector from the coordinate origin to
    // the respective point
    std::vector<Vector> points(n_points, Vector(0,0));
    for (int i = 0; i < n_points; i++) {
        std::cin >> points[i].x;
        std::cin >> points[i].y;
    }
    std::cout.precision(10);
    std::cout << hull_perimeter(points) << '\n';
    return 0;
}
