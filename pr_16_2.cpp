#include <iostream>
#include <vector>
#include <string>

std::vector<int> z_function(const std::string& str) {
    if (str.size() == 0) return std::vector<int>();
    std::vector<int> z_fn(str.size(), str.size());
    int l = 1;
    int r = 1;
    for (int i = 1; i < str.size(); i++) {
        if (z_fn[i - l] < r - i) {
            z_fn[z_fn[i - l]];
            continue;
        }
        l = i;
        r = i < r ? r : i;
        for (int j = r; j < str.size(); j++) {
            if (str[j] ==  str[j - l]) {
                r++;
            } else {
                break;
            }
        }
        z_fn[i] = r - l;
    }
    return z_fn;
}

std::vector<int> find_substring(const std::string& substr, const std::string& str) {
    if (substr.size() == 0) return std::vector<int>();
    if (substr.size() > str.size()) return std::vector<int>();
    std::vector<int> z_fn = z_function(substr);
    std::vector<int> match_indices;
    int l = 0;
    int r = 0;
    for (int i = 0; i < str.size() - substr.size() + 1; i++) {
        if (z_fn[(i - l) >= substr.size() ? 0 : i - l] < r - i) continue;
        l = i;
        r = i < r ? r : i;
        for (int j = r; j < l + substr.size(); j++) {
            if (str[j] ==  substr[j - l]) {
                r++;
            } else {
                break;
            }
        }
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
