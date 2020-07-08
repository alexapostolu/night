#include <iostream>
#include "Vector2D.h"

int main()
{
    ice::Vector2D w(5, 3.0);
    std::cout << w.mag() << '\n';
    w.norm();
    std::cout << w.mag();
}