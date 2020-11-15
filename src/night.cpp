#include "../include/night.h"
#include "../include/token.h"

#include <iostream>

void NightPrint(const Expression& value)
{
    if (value.type == ValueType::BOOL_ARR || value.type == ValueType::NUM_ARR || value.type == ValueType::STRING_ARR)
    {
        std::cout << "[ ";
        for (std::size_t a = 0; a < value.extras.size() - 1; ++a)
        {
            NightPrint(value.extras[a]);
            std::cout << ", ";
        }
        std::cout << value.extras.back().data << " ]";
    }
    else
    {
        std::cout << value.data;
    }

    std::cout.flush();
}