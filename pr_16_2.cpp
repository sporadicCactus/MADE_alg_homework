#include <iostream>
#include <vector>
#include <string>


std::vector<int> find_substring(std::string substr, std::string str) {
    if (substr.size() == 0) return std::vector<int>();
    if (substr.size() > str.size()) return std::vector<int>();
    std::vector<int> z_function;
    z_function.push_back(substr.size());
    int l = 1;
    int r = 1;
    for (int i = 1; i < substr.size(); i++) {
        if (i >= r) {
            l = i;
            r = i;
            for (int j = l; j < substr.size(); j++) {
                if (substr.at(j) == substr.at(j - l)) {
                    r++;
                } else {
                    break;
                }
            }
            z_function.push_back(r - l);
            continue;
        }
        if (z_function.at(i - l) < r - i) {
            z_function.push_back(z_function.at(i-l));
            continue;
        }
        l = i;
        for (int j = r; j < substr.size(); j++) {
            if (substr.at(j) == substr.at(j - l)) {
                r++;
            } else {
                break;
            }
        }
        z_function.push_back(r - l);
    }
    std::vector<int> match_indices;
    l = 0;
    r = 0;
    for (int i = 0; i < str.size() - substr.size() + 1; i++) {
        if (i >= r) {
            l = i;
            r = i;
            for (int j = l; j < l + substr.size(); j++) {
                if (str.at(j) == substr.at(j - l)) {
                    r++;
                } else {
                    break;
                }
            }
            z_function.push_back(r - l);
            if ((r - l) == substr.size()) match_indices.push_back(i);
            continue;
        }
        if (z_function.at(i - l) < r - i) {
            z_function.push_back(z_function.at(i - l));
            continue;
        }
        l = i;
        for (int j = r; (j < l + substr.size()) && (j < str.size()); j++) {
            if (str.at(j) == substr.at(j - l)) {
                r++;
            } else {
                break;
            }
        }
        z_function.push_back(r - l);
        if ((r - l) == substr.size()) match_indices.push_back(i);
    }
    return match_indices;
}

int main() {
    std::string substr;
    std::cin >> substr;
    std::string str;
    std::cin >> str;
    std::vector<int> match_indices = find_substring(substr, str);
    for (int i = 0; i < match_indices.size(); i++) {
        std::cout << match_indices.at(i) << ' ';
    }
    std::cout << '\n';
    return 0;
}
