#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

struct Bytecode;

using bytecodes_t = std::vector<std::shared_ptr<Bytecode>>;

enum class BytecodeType
{
	VARIABLE,
	VALUE
};

struct Bytecode
{
	BytecodeType type;
};

struct BytecodeVariable : public Bytecode
{
	std::string name;
};

struct BytecodeValue : public Bytecode
{
	std::string name;
};