#pragma once

#include "token.h"
#include "error.h"

#include <string>
#include <vector>

// splits an array of tokens into different statements
std::vector<std::vector<Token> > SplitCode(const std::vector<Token>& tokens);

// variable type to string
std::string VarTypeToStr(const VariableType& type);

// used in interpreter.h
// finds variable or function in array and returns it's address
template <typename T>
T* GetContainer(std::vector<T>& container, const std::string& token);

// used in parser.h
// returns default value for a given type
std::string DefaultValue(const ValueType& type);

// index starts right after opening bracket; advances index to closing bracket
template <typename Unit, typename UnitType>
void AdvanceCloseBracketIndex(const std::string& file, int line, const std::vector<Unit>& units,
    const UnitType& openBracket, const UnitType& closeBracket, std::size_t& index);