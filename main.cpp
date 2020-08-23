#include <algorithm>
#include <iostream>
#include <iterator>
#include <memory>
#include <ostream>
#include <utility>
#include <vector>
#include <fstream>
#include "include/vector.h"
#include "tests/test_type.h"

int main() {
    std::vector<trace_int> v;
    v.emplace_back(1);
    v.emplace_back(2);
    v.emplace_back(3);
    v.shrink_to_fit();
    v.assign(3, v.front());
    for (auto e : v) std::cout << e << " ";
}
