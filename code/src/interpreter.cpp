#include "interpreter.hpp"

#include <assert.h>
#include <stdexcept>

Interpreter::Interpreter(bytecodes_t const& _bytecodes)
	: bytecodes(_bytecodes) {}

void Interpreter::parse_bytecode()
{
	expr_stack s;

	for (auto const& code : bytecodes)
	{
		switch (code.type)
		{
		case BytecodeType::CONSTANT: s.push({ false, code.val }); break;
		case BytecodeType::VARIABLE: s.push({ true,  code.val }); break;

		case BytecodeType::NOT:  s.push({ false, !pop(s)		 }); break;
		case BytecodeType::ADD:  s.push({ false, pop(s) + pop(s) }); break;
		case BytecodeType::SUB:  s.push({ false, pop(s) - pop(s) }); break;
		case BytecodeType::MULT: s.push({ false, pop(s) * pop(s) }); break;
		case BytecodeType::DIV:  s.push({ false, pop(s) / pop(s) }); break;

		case BytecodeType::ASSIGN:
			assert_assignment();
			scope.vars[code.val] = pop(s);
			break;

		case BytecodeType::ADD_ASSIGN:
			assert_assignment();
			scope.vars[code.val] += pop(s);
			break;

		case BytecodeType::SUB_ASSIGN:
			assert_assignment();
			scope.vars[code.val] -= pop(s);
			break;

		case BytecodeType::MULT_ASSIGN:
			assert_assignment();
			scope.vars[code.val] *= pop(s);
			break;

		case BytecodeType::DIV_ASSIGN:
			assert_assignment();
			scope.vars[code.val] /= pop(s);
			break;

		case BytecodeType::IF:
			break;
		case BytecodeType::ELIF:
			break;
		case BytecodeType::ELSE:
			break;
		case BytecodeType::END_IF:
			break;

		default:
			throw std::runtime_error("Interpreter::parse_bytecodes unhandled case " + bytecode_to_str(code.type));
		}
	}
}

int Interpreter::pop(expr_stack& s)
{
	assert(!s.empty() && "u momma gay");

	auto [flag, val] = s.top();
	s.pop();

	return flag ? scope.vars[val] : val;
}