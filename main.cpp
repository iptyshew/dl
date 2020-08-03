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
    dl::vector<Custom> v;
    Custom a, b;
    v.push_back(a);
    v.push_back(b);
}
