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

struct NightClass
{
    std::string name;
    std::vector<NightVariable> variables;
    std::vector<NightVariable> methods;
};

// displays standard output
void NightPrint(const Expression& value);