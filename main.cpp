#include <iostream>
#include <vector>
#include "vector.h"

int main()
{
    dl::vector<int> vec = {9, 7, 4};
    vec.push_back(19);
    for (int e : vec) {
        std::cout << e << std::endl;
    }
}
