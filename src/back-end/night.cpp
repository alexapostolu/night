#include "../../include/back-end/night.hpp"
#include "../../include/back-end/token.hpp"

#include <iostream>
#include <cassert>

bool NightData::is_num() const
{
    return type == VariableType::INT || type == VariableType::FLOAT;
}

float NightData::get_num() const
{
    return type == VariableType::INT
        ? std::get<int>(data)
        : std::get<float>(data);
}

std::string NightData::to_str() const
{
    switch (type.type)
    {
    case VariableType::BOOL:
        return std::get<bool>(data) ? "true" : "false";
    case VariableType::INT:
        return std::to_string(std::get<int>(data));
    case VariableType::FLOAT:
        return std::to_string(std::get<float>(data));
    case VariableType::STR:
        return std::get<std::string>(data);
    default:
        assert(false);
    }
}

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
        std::cout << value.to_str();
    }

    std::cout.flush();
}