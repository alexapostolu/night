#include "bytecode.hpp"
#include "value.hpp"
#include "interpreter.hpp"

#include <stdexcept>
#include <variant>
#include <string>
#include <assert.h>

std::string Bytecode::to_str() const
{
	return bytecode_to_str(type);
}

std::string bytecode_to_str(BytecodeType type)
{
	switch (type)
	{
	case BytecodeType::CONSTANT:
		return "CONSTANT";
	case BytecodeType::VARIABLE:
		return "VARIABLE";

	case BytecodeType::NOT:
		return "NOT";
	case BytecodeType::ADD:
		return "ADD";
	case BytecodeType::SUB:
		return "SUB";
	case BytecodeType::MULT:
		return "MULT";
	case BytecodeType::DIV:
		return "DIV";

	case BytecodeType::ADD_ASSIGN:
		return "ADD_ASSIGN";
	case BytecodeType::SUB_ASSIGN:
		return "SUB_ASSIGN";
	case BytecodeType::MULT_ASSIGN:
		return "MULT_ASSIGN";
	case BytecodeType::DIV_ASSIGN:
		return "DIV_ASSIGN";

	case BytecodeType::IF:
		return "IF";
	case BytecodeType::ELIF:
		return "ELIF";
	case BytecodeType::ELSE:
		return "ELSE";

	case BytecodeType::RETURN:
		return "RETURN";

	default:
		throw std::runtime_error("Bytecode::to_str unhandled case " + (int)type);
	}
}