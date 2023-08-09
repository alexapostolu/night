#include "interpreter.hpp"
#include "interpreter_scope.hpp"
#include "error.hpp"
#include "debug.hpp"

#include <iostream>
#include <cmath>
#include <stack>
#include <optional>
#include <assert.h>

std::optional<intpr::Value> interpret_bytecodes(InterpreterScope& scope, bytecodes_t const& codes)
{
	std::stack<intpr::Value> s;

	for (auto it = std::cbegin(codes); it != std::cend(codes); ++it)
	{
		switch ((BytecodeType)*it)
		{
		case BytecodeType::BOOL:
			s.emplace((int64_t)*(++it));
			break;
		case BytecodeType::CHAR1:
			s.emplace((int64_t)*(++it));
			break;

		case BytecodeType::S_INT1:
		case BytecodeType::S_INT2:
		case BytecodeType::S_INT4:
		case BytecodeType::S_INT8:
			s.emplace(get_int<int64_t>(it));
			break;

		case BytecodeType::U_INT1:
		case BytecodeType::U_INT2:
		case BytecodeType::U_INT4:
		case BytecodeType::U_INT8:
			s.emplace(get_int<uint64_t>(it));
			break;

		case BytecodeType::FLOAT4:
		case BytecodeType::FLOAT8:
			push_float(s, it);
			break;

		case BytecodeType::STR:
			push_str(s, it);
			break;

		case BytecodeType::NEGATIVE: s.emplace(- pop(s).i); break;
		case BytecodeType::NOT:		 s.emplace((int64_t)!pop(s).i); break;

		case BytecodeType::ADD:	 s.emplace(pop(s).i + pop(s).i); break;
		case BytecodeType::MULT: s.emplace(pop(s).i * pop(s).i); break;
		case BytecodeType::DIV:  s.emplace(pop(s).i / pop(s).i); break;
		case BytecodeType::SUB:  s.emplace(pop(s).i - pop(s).i); break;

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

int64_t get_sint(bytecodes_t::const_iterator& it)
{
	int count;

	switch ((BytecodeType)(*it))
	{
	case BytecodeType::S_INT1: count = 8;  break;
	case BytecodeType::S_INT2: count = 16; break;
	case BytecodeType::S_INT4: count = 32; break;
	case BytecodeType::S_INT8: count = 64; break;
	default: throw debug::unhandled_case(*it);
	}

	int64_t num = *(++it);
	for (int i = 8; i < count; i *= 2)
	{
		auto byte = *(++it);
		num += byte * (pow(2, i));
	}

	return num;
}

uint64_t get_uint(bytecodes_t::const_iterator& it)
{
	int count;

	switch ((BytecodeType)(*it))
	{
	case BytecodeType::U_INT1: count = 8;  break;
	case BytecodeType::U_INT2: count = 16; break;
	case BytecodeType::U_INT4: count = 32; break;
	case BytecodeType::U_INT8: count = 64; break;
	default: throw debug::unhandled_case(*it);
	}

	uint64_t num = *(++it);
	for (int i = 8; i < count; i *= 2)
	{
		auto byte = *(++it);
		num += byte * (pow(2, i));
	}

	return num;
}

void push_float(std::stack<intpr::Value>& s, bytecodes_t::const_iterator& it)
{
	switch ((BytecodeType)(*(it++)))
	{
	case BytecodeType::FLOAT4:
		break;
	case BytecodeType::FLOAT8:
		break;
	default:
		throw debug::unhandled_case(*it);
	}
}

void push_str(std::stack<intpr::Value>& s, bytecodes_t::const_iterator& it)
{
	assert(*it == (bytecode_t)BytecodeType::STR);

	std::string str;
	int64_t size = get_int<int64_t>(++it);

	for (int i = 0; i < size; ++i)
	{
		++it;
		str += *it;
	}

	s.emplace(str);
}

intpr::Value pop(std::stack<intpr::Value>& s)
{
	auto val = s.top();
	s.pop();

	return val;
}