#include "../include/night.hpp"
#include "../include/token.hpp"

#include <iostream>

void NightPrint(const NightData& value)
{
    if (value.type != VariableType::BOOL && value.type != VariableType::NUM && value.type != VariableType::STR)
    {
        std::cout << "[ ";
        for (int a = 0; a < (int)value.extras.size() - 1; ++a)
        {
            NightPrint(value.extras[a]);
            std::cout << ", ";
        }
        
        if (value.extras.empty())
        {
            std::cout << "]";
        }
        else
        {
            NightPrint(value.extras.back());
            std::cout << " ]";
        }
    }
    else
    {
        std::cout << value.data;
    }

    std::cout.flush();
}