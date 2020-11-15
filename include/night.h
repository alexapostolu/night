#pragma once

#include "token.h"

#include <string>
#include <vector>

struct NightVariable
{
    std::string name;
    Expression data;
};

struct NightFunction
{
    std::string name;
    std::vector<std::string> parameters;
    std::vector<Statement> body;
};

// displays standard output
void NightPrint(const Expression& value);