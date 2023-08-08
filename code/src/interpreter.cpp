#include "interpreter.hpp"
#include "interpreter_scope.hpp"
#include "error.hpp"
#include "debug.hpp"

#include <iostream>
#include <stack>
#include <string.h>
#include <cmath>

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
			push_int(s, it);

		case BytecodeType::FLOAT4:
		case BytecodeType::FLOAT8:
			push_num(s, it);
			break;

		case BytecodeType::NEGATIVE: s.emplace(intpr::ValueType::INT, -pop(s).i); break;
		case BytecodeType::NOT:		 s.emplace(intpr::ValueType::INT, !pop(s).i); break;

		case BytecodeType::ADD: {
			auto v1 = pop(s);
			auto v2 = pop(s);
			if (v1.type == intpr::ValueType::STR)
				s.emplace(v1.s + v2.s);
			else
				s.emplace(intpr::ValueType::INT, v1.i + v2.i);
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

				for (int i = 0; i < scope.funcs[id].param_ids.size(); ++i)
					func_scope.vars[InterpreterScope::funcs[id].param_ids[i]] = pop(s);

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

void push_int(std::stack<intpr::Value>& s, bytecodes_t::const_iterator& it)
{
	int count;
	bool neg;

	switch ((BytecodeType)(*it))
	{
	case BytecodeType::S_INT1: count = 8; neg = true; break;
	case BytecodeType::S_INT2: count = 16; neg = true; break;
	case BytecodeType::S_INT4: count = 32; neg = true; break;
	case BytecodeType::S_INT8: count = 64; neg = true; break;
	case BytecodeType::U_INT1: count = 8; neg = false; break;
	case BytecodeType::U_INT2: count = 16; neg = false; break;
	case BytecodeType::U_INT4: count = 32; neg = false; break;
	case BytecodeType::U_INT8: count = 64; neg = false; break;
	default: throw debug::unhandled_case(*it);
	}

	if (neg)
	{
		int64_t num = *(++it);
		for (int i = 8; i < count; i *= 2)
		{
			auto byte = *(++it);
			num += byte * (pow(2, i));
		}
	}
	else
	{
		uint64_t num = *(++it);
		for (int i = 8; i < count; i *= 2)
		{
			auto byte = *(++it);
			num += byte * (pow(2, i));
		}
	}
}

void push_num(std::stack<intpr::Value>& s, bytecodes_t::const_iterator& it)
{
	switch ((BytecodeType)(*(it++)))
	{
	case BytecodeType::S_INT1:
		s.emplace(intpr::ValueType::INT, *it);
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