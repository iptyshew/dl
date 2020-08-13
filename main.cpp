#include <algorithm>
#include <iostream>
#include <iterator>
#include <memory>
#include <ostream>
#include <utility>
#include <vector>
#include <fstream>
#include "include/vector.h"

int main() {
    std::vector<int> v{1, 2, 3, 4, 5};
    v.reserve(7);
    auto b = v;
    std::cout << b.capacity();
}
