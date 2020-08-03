#include <iostream>
#include <memory>
#include <vector>
#include "vector.h"

class Custom
{
public:
    Custom(int a) : a(a) {}
    int a;
};

int main() {
    dl::vector<int> v;
    v.push_back(1);
    std::cout << sizeof(v);
}
