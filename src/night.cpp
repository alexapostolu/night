#include "../include/night.hpp"
#include "../include/token.hpp"

#include <iostream>

void NightPrint(const NightData& value)
{
    if (value.type == VariableType::ARRAY)
    {
        std::cout << "[ ";
        for (int a = 0; a < (int)value.extras.size() - 1; ++a)
        {
            NightPrint(value.extras[a]);
            std::cout << ", ";
        }

        if (!value.extras.empty())
            NightPrint(value.extras.back());
        
        std::cout << " ]";
    }
    else
    {
        std::cout << value.data;
    }

    std::cout.flush();
}