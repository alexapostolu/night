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
        return {};
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

bool compare_array(const NightData& arr1, const NightData& arr2)
{
    if (arr1.extras.size() != arr2.extras.size())
        return false;

    for (std::size_t a = 0; a < arr1.extras.size(); ++a)
    {
        if (arr1.extras[a].type != arr2.extras[a].type)
            return false;
        if (arr1.extras[a].type == VariableType::ARRAY && !compare_array(arr1.extras[a], arr2.extras[a]))
            return false;
    }

    return true;
}

NightScope::NightScope(const std::shared_ptr<NightScope>& _upper_scope)
    : upper_scope(_upper_scope)
{
}