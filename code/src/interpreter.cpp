#include "interpreter.hpp"
#include "interpreter_scope.hpp"
#include "error.hpp"

#include <assert.h>

Interpreter::Interpreter(bytecodes_t& _codes)
	: codes(_codes) {}

void Interpreter::interpret_bytecodes()
{
	int passed_conditionals = 0;
	for (auto it = std::cbegin(codes); it != std::cend(codes); ++it)
	{
		switch ((BytecodeType)*it)
		{
		case BytecodeType::BOOL:
			push_bool(it);
			break;
		case BytecodeType::CHAR1:
			push_char(it);
			break;

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
			push_num(it);
			break;

		case BytecodeType::LOAD:
			push_var(it);
			break;

		case BytecodeType::NEGATIVE: s.push(-pop()); break;
		case BytecodeType::NOT:		 s.push(pop() == 0); break;

		case BytecodeType::ADD:  s.push(pop() + pop()); break;
		case BytecodeType::SUB:  s.push(pop() - pop()); break;
		case BytecodeType::MULT: s.push(pop() * pop()); break;
		case BytecodeType::DIV:  s.push(pop() / pop()); break;

		case BytecodeType::STORE:
			vars[pop()] = pop();
			break;

		case BytecodeType::JUMP:
			auto line = pop();
			std::advance(it, line);
			break;
		case BytecodeType::JUMP_IF_FALSE:
			auto line = pop();
			std::advance(it, line);

		case BytecodeType::WHILE:
			break;
		case BytecodeType::FOR:
			break;

		case BytecodeType::RETURN:
			break;
		case BytecodeType::FUNC_CALL:
			break;

		default:
			night::throw_unhandled_case(*it);
		}
	}
}

void Interpreter::push_bool(bytecodes_t::const_iterator& it)
{
	++it;
	s.push(*it != 0);
}

void Interpreter::push_num(bytecodes_t::const_iterator& it)
{
	auto type = (BytecodeType)(*it);
	++it;

	switch (type)
	{
	case BytecodeType::S_INT1:
		s.push(-(*it));
		break;
	case BytecodeType::S_INT2:
	case BytecodeType::S_INT4:
	case BytecodeType::S_INT8:
	case BytecodeType::U_INT1:
		s.push(*it);
		break;
	case BytecodeType::U_INT2:
	case BytecodeType::U_INT4:
	case BytecodeType::U_INT8:
	case BytecodeType::FLOAT4:
	case BytecodeType::FLOAT8:
	default:
		night::throw_unhandled_case(*it);
	}
}

void Interpreter::push_char(bytecodes_t::const_iterator& it)
{
	++it;

	switch ((BytecodeType)*it)
	{
	case BytecodeType::CHAR1:
		s.push(*it);
		break;
	default:
		night::throw_unhandled_case(*it);
	}
}

void Interpreter::push_var(bytecodes_t::const_iterator& it)
{
	++it;
	s.push(vars[*it]);
}

int Interpreter::pop()
{
	auto val = s.top();
	s.pop();

	return val;
}