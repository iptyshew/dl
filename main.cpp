#include <algorithm>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>
#include "include/vector.h"


int main() {
    std::vector<int> a;

    a.resize(3);


    a.reserve(4);
    a.insert(a.end(), {1, 2, 3});
    std::cout << a. capacity();
}
