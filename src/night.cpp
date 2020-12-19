#include "../include/night.hpp"
#include "../include/token.hpp"

#include <iostream>

void NightPrint(const NightData& value)
{
    if (!value.extras.empty())
    {
        std::cout << "[ ";
        for (std::size_t a = 0; a < value.extras.size() - 1; ++a)
        {
            NightPrint(value.extras[a]);
            std::cout << ", ";
        }
        
        NightPrint(value.extras.back());
        std::cout << " ]";
    }
    else
    {
        std::cout << value.data;
    }

    std::cout.flush();
}