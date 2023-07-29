#include "interpreter.hpp"
#include "interpreter_scope.hpp"
#include "error.hpp"

func_container Interpreter::funcs;

void interpret_bytecodes(bytecodes_t const& codes)
{
	Interpreter i;

	for (auto it = std::cbegin(codes); it != std::cend(codes); ++it)
	{
		switch ((BytecodeType)*it)
		{
		case BytecodeType::BOOL:
			i.push_bool(it);
			break;
		case BytecodeType::CHAR1:
			i.push_char(it);
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
			i.push_num(it);
			break;

		case BytecodeType::NEGATIVE: i.s.push(-i.pop()); break;
		case BytecodeType::NOT:		 i.s.push(!i.pop()); break;

		case BytecodeType::ADD:  i.s.push(i.pop() + i.pop()); break;
		case BytecodeType::SUB:  i.s.push(i.pop() - i.pop()); break;
		case BytecodeType::MULT: i.s.push(i.pop() * i.pop()); break;
		case BytecodeType::DIV:  i.s.push(i.pop() / i.pop()); break;

		case BytecodeType::LOAD:
			++it;
			i.s.push(i.vars[*it]);
			break;

		case BytecodeType::STORE:
			i.vars[*(++it)] = i.pop();
			break;

		case BytecodeType::JUMP_IF_FALSE:
			if (!i.pop())
				std::advance(it, *(++it));
			break;

		case BytecodeType::JUMP:
			std::advance(it, *(++it));
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