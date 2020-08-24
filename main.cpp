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
#include "include/memory.h"

int main() {
    dl::unique_ptr<int> a;
    std::cout << *a;
}
