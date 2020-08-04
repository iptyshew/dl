#include <algorithm>
#include <iostream>
#include <memory>
#include <vector>
#include "vector.h"

class Custom
{
public:
    Custom() = default;
    ~Custom() {
        std::cout << "destroy\n" ;
    }
};

template<typename T>
void print(const dl::vector<T>& v) {
    for (auto&& e : v) {
        std::cout << e << " ";
    }
    std::cout << std::endl;
}

int main() {
    std::vector<int> v;
    v.insert(v.begin(), 1);
    std::cout << v.capacity() << " ";
    v.insert(v.begin(), 1);
    std::cout << v.capacity() << " ";
    v.insert(v.begin(), 1);
    std::cout << v.capacity() << " ";
}
