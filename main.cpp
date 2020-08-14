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
    std::vector<int> a{1, 2, 3};
    a.resize(4);
    std::cout << a.capacity();
    std::sort(a.begin(), a.end());

}
