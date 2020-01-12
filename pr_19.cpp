#include <iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <queue>
#include <tuple>
#include <cmath>

typedef unsigned long long int ui;

ui MAX_NUM = ~0;
double X_BOUND = 180.;
double Y_BOUND = 90.;
int MAX_DEPTH = 2;
int MAX_POINTS_IN_NODE = 4;

enum RelPos {Inside, Outside, OnBorder};

struct Point {
    ui x;
    ui y;
    Point(ui x_, ui y_): x(x_), y(y_) {}
    Point(double x_, double y_) {
        x = (ui)std::round((x_ + X_BOUND)/(2*X_BOUND)*MAX_NUM);
        y = (ui)std::round((y_ + Y_BOUND)/(2*Y_BOUND)*MAX_NUM);
    }
};

struct HashNode {
    std::vector<HashNode*> children;
    std::vector<Point> points;
    int n_points;
    Point lower_left;
    Point upper_right;
    HashNode(Point ll, Point ur): n_points(0), lower_left(ll), upper_right(ur) {}
};

class HashTree {
    private:
    HashNode* root;

    int hash(Point p, int depth) {
        if (depth % 2 == 0) {
            int x_hash = (p.x << ((depth/2)*5)) >> (8*sizeof(ui) - 3);
            int y_hash = (p.y << ((depth/2)*5)) >> (8*sizeof(ui) - 2);
            return x_hash + 8*y_hash;
        } else {
            int x_hash =  (p.x << ((depth/2)*5 + 3)) >> (8*sizeof(ui) - 2);
            int y_hash =  (p.y << ((depth/2)*5 + 2)) >> (8*sizeof(ui) - 3);
            return x_hash + 4*y_hash;
        }
    }
    void spawn_nodes(HashNode* node, int depth) {
        ui dx = node->upper_right.x - node->lower_left.x;
        ui dy = node->upper_right.y - node->lower_left.y;
        int divx = depth % 2 == 0 ? 8 : 4;
        int divy = depth % 2 == 0 ? 4 : 8;
        for (int i = 0; i < 32; i++) {
            Point new_lower_left = node->lower_left;
            new_lower_left.x += (dx / divx) * (i % divx);
            new_lower_left.y += (dy / divy) * (i / divx);
            Point new_upper_right = new_lower_left;
            new_upper_right.x += dx / divx;
            new_upper_right.y += dy / divy;
            HashNode* new_node = new HashNode(new_lower_left, new_upper_right);
            node->children.push_back(new_node);
        }
        for (int i = 0; i < node->points.size(); i++) {
            Point pt = node->points[i];
            int pt_hash = hash(pt, depth);
            node->children[pt_hash]->points.push_back(pt);
            node->children[pt_hash]->n_points++;
        }
        node->points.clear();
    }
    RelPos compare_node_to_box(HashNode* node, Point lower_left, Point upper_right) {
        if ((node->lower_left.x >= upper_right.x) || (node->lower_left.y >= upper_right.y)) return Outside;
        if ((node->upper_right.x <= lower_left.x) || (node->upper_right.y <= lower_left.y)) return Outside;
        if ((node->lower_left.x >= lower_left.x) && (node->lower_left.y >= lower_left.y) &&
            (node->upper_right.x <= upper_right.x) && (node->upper_right.y <= upper_right.y)) return Inside;
        return OnBorder;
    }
    bool point_in_box(Point p, Point lower_left, Point upper_right) {
        bool res = true;
        res = res && (p.x >= lower_left.x);
        res = res && (p.y >= lower_left.y);
        res = res && (p.x < upper_right.x);
        res = res && (p.y < upper_right.y);
        return res;
    }


    public:
    HashTree() {
        root = new HashNode(Point((ui)0,(ui)0), Point(MAX_NUM, MAX_NUM));
    }
    ~HashTree() {
        std::queue<HashNode*> queue;
        queue.push(root);
        while (queue.size() > 0) {
            HashNode* current = queue.front();
            queue.pop();
            for (int i = 0; i < current->children.size(); i++) {
                queue.push(current->children[i]);
            }
            delete current;
        }
    }
    HashTree(const HashTree& other) = delete;
    HashTree& operator=(const HashTree& other) = delete;
    HashTree(HashTree&& other) = delete;
    HashTree& operator=(HashTree&&) = delete;
    void AddPoint(Point pt) {
        int depth = 0;
        HashNode* current_node = root;
        while (current_node->children.size() != 0) {
            current_node->n_points++;
            current_node = current_node->children[hash(pt, depth)];
            depth++;
        }
        current_node->n_points++;
        current_node->points.push_back(pt);
        if ((current_node->points.size() > MAX_POINTS_IN_NODE) && (depth < MAX_DEPTH)) {
            spawn_nodes(current_node, depth);
        }
    }
    int CountPointsInBox(Point lower_left, Point upper_right) {
        int counter = 0;
        std::queue<HashNode*> queue;
        queue.push(root);
        while (queue.size() > 0) {
            HashNode* current = queue.front();
            queue.pop();
            RelPos rel_pos = compare_node_to_box(current, lower_left, upper_right);
            if (rel_pos == Inside) {
                counter += current->n_points;
                continue;
            }
            if (rel_pos == Outside) continue;
            for (int i = 0; i < current->children.size(); i++) {
                queue.push(current->children[i]);
            }
            if (current->children.size() > 0) continue;
            for (int i = 0; i < current->points.size(); i++) {
                if (point_in_box(current->points[i], lower_left, upper_right)) counter++;
            }
        }
        return counter;
    }
};

int main() {
    HashTree hash_tree;
    std::ifstream input("input.txt");
    std::ofstream output("output.txt");
    int n_points;
    input >> n_points;
    for (int i = 0; i < n_points; i++) {
        double x_coord;
        input >> x_coord;
        double y_coord;
        input >> y_coord;
        hash_tree.AddPoint(Point(x_coord, y_coord));
    }
    int n_boxes;
    input >> n_boxes;
    for (int i = 0; i < n_boxes; i++) {
        double x_ll;
        input >> x_ll;
        double y_ll;
        input >> y_ll;
        double x_ur;
        input >> x_ur;
        double y_ur;
        input >> y_ur;
        std::cout << hash_tree.CountPointsInBox(Point(x_ll, y_ll), Point(x_ur, y_ur)) << '\n';
    }
    return 0;
}