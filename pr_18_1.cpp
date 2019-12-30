#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <cmath>
#include <stack>

double EPSILON = 0.000000001;

struct Point {
    double x;
    double y;
    Point(double x_, double y_): x(x_), y(y_) {}
    double dot(Point pt) const {
        return x*pt.x + y*pt.y;
    }
    double cross(Point pt) const {
        return x*pt.y - y*pt.x;
    }
    double len() const {
        return std::sqrt(x*x + y*y);
    }
    Point operator-(Point other) const {
        return Point(x - other.x, y - other.y);
    }
    Point operator+(Point other) const {
        return Point(x + other.x, y + other.y);
    }
    bool operator<(Point other) const {
        if (x != other.x) return x < other.x;
        return y < other.y;
    }
};

double hull_perimeter(const std::vector<Point>& points) {
    if (points.size() < 2) return 0.;
    if (points.size() == 2) {
        return 2*(points[0] - points[1]).len();
    }
    Point start = points[0];
    int start_ind = 0;
    double accumulator = 0;
    for (int i = 1; i < points.size(); i++) {
        accumulator += std::abs(points[i].x) + std::abs(points[i].y);
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
    accumulator /= points.size();
    accumulator *= EPSILON;
    start = start - Point(accumulator, 0.);
    std::priority_queue<std::pair<double, Point> > points_queue;
    for (int i = 0; i < points.size(); i++) {
        if (i == start_ind) continue;
        double sin = (points[i] - start).cross(Point(1., 0.)) / (points[i] - start).len();
        points_queue.push(std::make_pair(sin, points[i]));
    }
    std::vector<Point> boundary;
    boundary.push_back(start);
    Point prev_edge(0,0);
    while (points_queue.size() > 0) {
        Point top_point = points_queue.top().second;
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
        if (boundary.size() < 2) {
            boundary.push_back(top_point);
            prev_edge = top_point - start;
            continue;
        }
        Point new_edge = top_point - boundary.back();
        bool left_turn = (prev_edge.cross(new_edge) > 0.);
        while (!left_turn) {
            boundary.pop_back();
            if (boundary.size() < 2) continue;
            new_edge = top_point - boundary.back();
            prev_edge = boundary.back() - boundary[boundary.size() - 2];
            left_turn = (prev_edge.cross(new_edge) > 0.);
        }
        prev_edge = new_edge;
        boundary.push_back(top_point);
    }
    start = start + Point(accumulator, 0);
    boundary[0] = start;
    boundary.push_back(start);
    accumulator = 0;
    // std::cout << "Pt: " << start.x << ' ' << start.y << '\n';
    for (int i = 1; i < boundary.size(); i++) {
        accumulator += (boundary[i] - boundary[i - 1]).len();
        // std::cout << "Pt: " << boundary[i].x << ' ' << boundary[i].y << '\n';
    }
    return accumulator;
}

int main() {
    int n_points;
    std::cin >> n_points;
    std::vector<Point> points(n_points, Point(0,0));
    for (int i = 0; i < n_points; i++) {
        std::cin >> points[i].x;
        std::cin >> points[i].y;
    }
    std::cout.precision(10);
    std::cout << hull_perimeter(points) << '\n';
    return 0;
}
