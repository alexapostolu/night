#include "interpreter.hpp"

#include <assert.h>
#include <stdexcept>

// debugging with asserts
#define assert_assignment() \
	assert(scope.vars.contains(code->val) \
		&& "variable definitions should be checked in parser"); \
	assert(!s.empty() \
		&& "the stack should not be empty when assigning, should be checked in parser");

void interpret_bytecodes(Interpreter const& inperpreter, bytecodes_t const& codes)
{
	Function* func = nullptr;
	int func_params = 0;

	for (auto code = std::cbegin(codes); code != std::cend(codes); ++code)
	{
		switch (code->type)
		{
		case BytecodeType::S_INT1:
		case BytecodeType::S_INT2:
		case BytecodeType::S_INT4:
		case BytecodeType::S_INT8:
		case BytecodeType::U_INT1:
		case BytecodeType::U_INT2:
		case BytecodeType::U_INT4:
		case BytecodeType::U_INT8:
		case BytecodeType::FLOAT4:
		case BytecodeType::FLOAT8:
		case BytecodeType::VARIABLE:
			if (func_params > 0) 
			{
				--func_params;
				func->params.push_back(code->val);
			}
			else
			{
				s.push({ true,  code->val }); break;
			}

		case BytecodeType::NOT:  s.push({ false, !pop(s)		 }); break;
		case BytecodeType::ADD:  s.push({ false, pop(s) + pop(s) }); break;
		case BytecodeType::SUB:  s.push({ false, pop(s) - pop(s) }); break;
		case BytecodeType::MULT: s.push({ false, pop(s) * pop(s) }); break;
		case BytecodeType::DIV:  s.push({ false, pop(s) / pop(s) }); break;

		case BytecodeType::BOOL_ASSIGN:
			assert_assignment();
			scope.vars[code->val] = { ValueType::BOOL, pop(s) };
			break;
		case BytecodeType::CHAR_ASSIGN:
			assert_assignment();
			scope.vars[code->val] = { ValueType::CHAR, pop(s) };
			break;
		case BytecodeType::INT_ASSIGN:
			assert_assignment();
			scope.vars[code->val] = { ValueType::INT, pop(s) };
			break;

		case BytecodeType::ADD_ASSIGN:
			assert_assignment();
			scope.vars[code->val].val += pop(s);
			break;
		case BytecodeType::SUB_ASSIGN:
			assert_assignment();
			scope.vars[code->val].val -= pop(s);
			break;
		case BytecodeType::MULT_ASSIGN:
			assert_assignment();
			scope.vars[code->val].val *= pop(s);
			break;
		case BytecodeType::DIV_ASSIGN:
			assert_assignment();
			scope.vars[code->val].val /= pop(s);
			break;

		case BytecodeType::IF:
		case BytecodeType::ELIF:
			if (code->val)
			{
				interpret_bytecodes(bytecodes_t(++code, std::next(code, code->val + 1)));
				std::advance(code, code->val + 1);

				while (code != std::end(codes) && (code->type == BytecodeType::ELIF || code->type == BytecodeType::ELSE))
					std::advance(code, code->val + 1);
				--code;
			}
			else
			{
				std::advance(code, code->val);
			}
			break;
		case BytecodeType::ELSE:
			if (code->val)
				interpret_bytecodes(bytecodes_t(++code, std::next(code, code->val + 1)));
			else
				std::advance(code, code->val);
			break;

		case BytecodeType::FUNC:
			*func = Function{};
			func_params = code->val;
			break;

		default:
			throw std::runtime_error("Interpreter::parse_bytecodes unhandled case " + bytecode_to_str(code->type));
		}
	}
}

int pop(InterpreterScope& scope, expr_stack& s)
{
	assert(!s.empty() && "u momma gay");

	auto const& [flag, val] = s.top();
	s.pop();

	switch (type)
	{
		INT8,
			int16,
	}

	return flag ? scope.vars[val].val : val;
}