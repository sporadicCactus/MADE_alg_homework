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
int MAX_POINTS_IN_NODE = 16;

struct EndPoint {
    ui pt;
    int box;
    bool right;
    EndPoint(ui pt_, int box_, bool right_): pt(pt_), box(box_), right(right_) {}
};

struct HashNode {
    std::vector<HashNode*> children;
    std::unordered_set<int> internal;
    std::unordered_set<int> border;
};

struct Box {
    ui x_left;
    ui x_right;
    ui y_left;
    ui y_right;
    bool contains(std::pair<ui, ui> point) {
        bool x_match = (point.first >= x_left) && (point.first < x_right);
        bool y_match = (point.second >= y_left) && (point.second < y_right);
        return (x_match && y_match);
    }
    Box(ui a1, ui a2, ui a3, ui a4): x_left(a1), x_right(a3), y_left(a2), y_right(a4) {}
};

int inline hash(ui x, int depth) {
    return (x << (4 * depth)) >> (8*sizeof(ui) - 4);
}

class HashTree {
    private:
    bool empty;
    HashNode *x_root;
    HashNode *y_root;
    std::vector<std::vector<EndPoint> > divide_points(const std::vector<EndPoint>& points, int depth) {
        std::vector<std::vector<EndPoint> > out(16, std::vector<EndPoint>());
        for (int i = 0; i < points.size(); i++) {
            out[((points[i].pt) << 4 * depth) >> (8*sizeof(ui) - 4)].push_back(points[i]);
        }
        return out;
    }
    std::vector<std::vector<EndPoint> > populate_node(HashNode* node,
        const std::vector<EndPoint>& points, int depth) {
        std::vector<std::vector<EndPoint> > divided;
        if (points.size() < MAX_POINTS_IN_NODE || depth >= MAX_DEPTH) {
            for (int i = 0; i < points.size(); i++) {
                EndPoint point  = points[i];
                node->internal.erase(point.box);
                node->border.insert(point.box);
            }
        } else {
            for (int i = 0; i < 16; i++) {
                HashNode* new_node = new HashNode;
                new_node->internal = node->internal;
                node->children.push_back(new_node);
            }
            for (int i = 0; i < points.size(); i++) {
                EndPoint point  = points[i];
                int pos = hash(points[i].pt, depth);
                if (point.right) {
                    for (int j = pos + 1; j < 16; j++) {
                        node->children[j]->internal.erase(point.box);
                    }
                } else {
                    for (int j = 0; j < pos; j++ ) {
                        node->children[j]->internal.erase(point.box);
                    }
                }
                node->internal.clear();
            }        
            divided = divide_points(points, depth);
        }
        return divided;
    }
    HashNode* build_hash_tree(const std::vector<EndPoint>& EndPoints) {
        HashNode *root = new HashNode;
        for (int i = 0; i < EndPoints.size(); i++) {
            root->internal.insert(EndPoints[i].box);
        }
        std::queue<std::tuple<HashNode*, std::vector<EndPoint>, int> > build_queue;
        build_queue.push(std::make_tuple(root, EndPoints, 0));
        while (build_queue.size() > 0) {
            std::tuple<HashNode*, std::vector<EndPoint>, int>& current = build_queue.front();
            std::vector<std::vector<EndPoint> > divided = populate_node(std::get<0>(current),
                std::get<1>(current), std::get<2>(current));
            if (divided.size() == 0) {
                build_queue.pop();
                continue;
            }
            for (int i = 0; i < 16; i++) {
                build_queue.push(std::make_tuple(std::get<0>(current)->children[i],
                    divided[i], std::get<2>(current) + 1));
            }
            build_queue.pop();
        }
        return root;
    }
    void delete_subtree(HashNode *root) {
        
        std::queue<HashNode*> node_queue;
        node_queue.push(root);
        while(node_queue.size() > 0) {
            HashNode* current = node_queue.front();
            node_queue.pop();
            for (int i = 0; i < current->children.size(); i++) {
                node_queue.push(current->children[i]);
            }
            delete current;
        }
    }

    public:
    HashTree(): empty(true) {}
    ~HashTree() {
        if (!empty) {
            delete_subtree(x_root);
            delete_subtree(y_root);
        }
    }
    void Build(const std::vector<EndPoint>& endpoints_x, const std::vector<EndPoint>& endpoints_y) {
        if (!empty) return;
        x_root = build_hash_tree(endpoints_x);
        y_root = build_hash_tree(endpoints_y);
        empty = false;
    }
    std::pair<std::unordered_set<int>, std::unordered_set<int>>
        ProcessPoint(std::pair<ui, ui> point) {
        HashNode *current_x = x_root;
        int depth = 0;
        while (current_x->children.size() > 0) {
            current_x = current_x->children[hash(point.first, depth)];
            depth++;
        }
        HashNode *current_y = y_root;
        depth = 0;
        while (current_y->children.size() > 0) {
            current_y = current_y->children[hash(point.second, depth)];
            depth++;
        }
        std::unordered_set<int> certain;
        std::unordered_set<int> uncertain;
        std::unordered_set<int> *short_internal = &current_x->internal; 
        std::unordered_set<int> *long_internal = &current_y->internal;
        std::unordered_set<int> *short_border = &current_x->border; 
        std::unordered_set<int> *long_border = &current_y->border;
        if (short_internal->size() > long_internal->size()) {
            short_internal = &current_y->internal; 
            long_internal = &current_x->internal;
            short_border = &current_y->border; 
            long_border = &current_x->border;   
        } 
        for (int box_num : *short_internal) {
            if (long_internal->find(box_num) != long_internal->end()) {
                certain.insert(box_num);
            }
            if (long_border->find(box_num) != long_border->end()) {
                uncertain.insert(box_num);
            }
        }
        for (int box_num : *short_border) {
            if (long_internal->find(box_num) != long_internal->end()) {
                uncertain.insert(box_num);
            }
            if (long_border->find(box_num) != long_border->end()) {
                uncertain.insert(box_num);
            }
        }
        return std::make_pair(certain, uncertain);
    }
};

class Counter {
    private:
    std::vector<Box> boxes;
    HashTree hashtree;
    bool tree_built;
    std::vector<int> counter;

    public:
    Counter(): tree_built(false) {}
    void AddBox(Box new_box) {
        if (tree_built) return;
        boxes.push_back(new_box);
    }
    void Build() {
        std::vector<EndPoint> endpoints_x;
        std::vector<EndPoint> endpoints_y;
        for (int i = 0; i < boxes.size(); i++) {
            Box box = boxes[i];
            endpoints_x.push_back(EndPoint(box.x_left, i, false));
            endpoints_x.push_back(EndPoint(box.x_right, i, true));
            endpoints_y.push_back(EndPoint(box.y_left, i, false));
            endpoints_y.push_back(EndPoint(box.y_right, i, true));
        }
        hashtree.Build(endpoints_x, endpoints_y);
        counter = std::vector<int>(boxes.size(), 0);
        tree_built = true;
    }
    void AddPoint(std::pair<ui, ui> point) {
        if (!tree_built) return;
        std::pair<std::unordered_set<int>, std::unordered_set<int>>
            tree_output = hashtree.ProcessPoint(point);
        for (int box_num :  tree_output.first) {
            counter[box_num]++;
        }
        for (int box_num : tree_output.second) {
            if (boxes[box_num].contains(point)) counter[box_num]++;
        }
    }
    const std::vector<int>& GetCounts() const {
        return counter;
    } 
};

ui round_x(double x) {
    return (ui)std::round((x + X_BOUND)/(2*X_BOUND)*MAX_NUM);
}

ui round_y(double y) {
    return (ui)std::round((y + Y_BOUND)/(2*Y_BOUND)*MAX_NUM);
}

int main() {
    std::ifstream input("input.txt");
    std::ofstream output("output.txt");
    int n_points;
    input >> n_points;
    std::vector<std::pair<ui, ui> > points;
    for (int i = 0; i < n_points; i++) {
        double x_coord;
        input >> x_coord;
        double y_coord;
        input >> y_coord;
        ui int_x = round_x(x_coord);
        ui int_y = round_y(y_coord);
        points.push_back(std::make_pair(int_x, int_y));
    }
    Counter counter;
    int n_boxes;
    input >> n_boxes;
    for (int i = 0; i < n_boxes; i++) {
        double x_left;
        input >> x_left;
        double y_left;
        input >> y_left;
        double x_right;
        input >> x_right;
        double y_right;
        input >> y_right;
        counter.AddBox(Box(round_x(x_left), round_y(y_left), round_x(x_right), round_y(y_right)));
    }
    counter.Build();
    for (int i = 0; i < n_points; i++) {
        counter.AddPoint(points[i]);
    }
    const std::vector<int>& counts = counter.GetCounts();
    for (int i = 0; i < n_boxes; i++) {
        output << counts[i] << '\n';
    }
    return 0;
}
