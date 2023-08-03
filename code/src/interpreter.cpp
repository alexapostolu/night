#include "interpreter.hpp"
#include "interpreter_scope.hpp"
#include "error.hpp"
#include "debug.hpp"

#include <iostream>
#include <stack>
#include <string.h>

std::optional<intpr::Value> interpret_bytecodes(InterpreterScope& scope, bytecodes_t const& codes)
{
	std::stack<intpr::Value> s;

	for (auto it = std::cbegin(codes); it != std::cend(codes); ++it)
	{
		switch ((BytecodeType)*it)
		{
		case BytecodeType::BOOL:
			s.emplace(intpr::ValueType::INT, *(++it));
			break;
		case BytecodeType::CHAR1:
			s.emplace(intpr::ValueType::INT, *(++it));
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
			push_num(s, it);
			break;

		case BytecodeType::NEGATIVE: s.emplace(intpr::ValueType::INT, -pop(s).i); break;
		case BytecodeType::NOT:		 s.emplace(intpr::ValueType::INT, !pop(s).i); break;

		case BytecodeType::ADD: {
			auto t1 = pop(s);
			auto t2 = pop(s);
			if (t1.type == intpr::ValueType::STR)
				s.emplace(t1.s + t2.s);
			else
				s.emplace(intpr::ValueType::INT, pop(s).i + pop(s).i);
			break;
		}
		case BytecodeType::MULT: s.emplace(intpr::ValueType::INT, pop(s).i * pop(s).i); break;
		case BytecodeType::DIV:  s.emplace(intpr::ValueType::INT, pop(s).i / pop(s).i); break;
		case BytecodeType::SUB:  s.emplace(intpr::ValueType::INT, pop(s).i - pop(s).i); break;

		case BytecodeType::LOAD:
			++it;
			s.push(scope.vars[*it]);
			break;

		case BytecodeType::STORE:
			scope.vars[*(++it)] = pop(s);
			break;

		case BytecodeType::JUMP_IF_FALSE:
			if (!pop(s).i)
				std::advance(it, *(++it));
			break;

		case BytecodeType::JUMP:
			std::advance(it, *(++it));
			break;

		case BytecodeType::RETURN:
			return pop(s);

		case BytecodeType::CALL: {
			auto id = *(++it);

			switch (id)
			{
			case 0: std::cout << (bool)pop(s).i; break;
			case 1: std::cout << (char)pop(s).i; break;
			case 2: std::cout << (int)pop(s).i; break;
			case 3: std::cout << (float)pop(s).f; break;
			case 4: std::cout << pop(s).s; break;
			case 5: {
				std::string str;
				std::cin >> str;
				s.emplace(str);
				break;
			}
			default: {
				InterpreterScope func_scope{ scope.vars };

				for (int i = 0; i < scope.funcs[id].params.size(); ++i)
					func_scope.vars[InterpreterScope::funcs[id].params[i]] = pop(s);

				s.push(*interpret_bytecodes(func_scope, InterpreterScope::funcs[id].codes));

				break;
			}
			}

			break;
		}

		default:
			throw debug::unhandled_case(*it);
		}
	}

	return std::nullopt;
}

void push_num(std::stack<intpr::Value>& s, bytecodes_t::const_iterator& it)
{
	switch ((BytecodeType)(*(it++)))
	{
	case BytecodeType::S_INT1:
		s.emplace(intpr::ValueType::INT, -(*it));
		break;
	case BytecodeType::S_INT2:
	case BytecodeType::S_INT4:
	case BytecodeType::S_INT8:
	case BytecodeType::U_INT1:
		s.emplace(intpr::ValueType::INT, *it);
		break;
	case BytecodeType::U_INT2:
	case BytecodeType::U_INT4:
	case BytecodeType::U_INT8:
	case BytecodeType::FLOAT4:
	case BytecodeType::FLOAT8:
	default:
		throw debug::unhandled_case(*it);
	}
}

intpr::Value pop(std::stack<intpr::Value>& s)
{
	auto val = s.top();
	s.pop();

	return val;
}