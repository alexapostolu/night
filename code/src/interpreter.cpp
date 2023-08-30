#include "interpreter.hpp"
#include "interpreter_scope.hpp"
#include "error.hpp"
#include "debug.hpp"

#include <iostream>
#include <cmath>
#include <stack>
#include <optional>
#include <cstring>
#include <assert.h>

std::optional<intpr::Value> interpret_bytecodes(InterpreterScope& scope, bytecodes_t const& codes)
{
	std::stack<intpr::Value> s;

	for (auto it = std::begin(codes); it != std::end(codes); ++it)
	{
		switch ((BytecodeType)*it)
		{
		case BytecodeType::S_INT1:
		case BytecodeType::S_INT2:
		case BytecodeType::S_INT4:
		case BytecodeType::S_INT8:
			s.emplace(get_int<int64_t>(it));
			break;

		case BytecodeType::FLOAT4:
		case BytecodeType::FLOAT8:
			push_float(s, it);
			break;

		case BytecodeType::STR: push_str(s, it); break;
		case BytecodeType::ARR: push_arr(s, it); break;

		case BytecodeType::NEGATIVE_I:
			s.emplace(-pop(s).i);
			break;
		case BytecodeType::NEGATIVE_F:
			s.emplace(-pop(s).f);
			break;

		case BytecodeType::NOT_I:
			s.emplace((int64_t)!pop(s).i);
			break;
		case BytecodeType::NOT_F:
			s.emplace((int64_t)!pop(s).f);
			break;

		case BytecodeType::ADD_I:
			s.emplace(pop(s).i + pop(s).i);
			break;
		case BytecodeType::ADD_F:
			s.emplace(pop(s).f + pop(s).f);
			break;
		case BytecodeType::ADD_S: {
			auto s2 = pop(s);
			s.emplace(pop(s).s + s2.s);
			break;
		}

		case BytecodeType::SUB_I:
			s.emplace(-pop(s).i + pop(s).i);
			break;
		case BytecodeType::SUB_F:
			s.emplace(-pop(s).f + pop(s).f);
			break;

		case BytecodeType::MULT_I:
			s.emplace(pop(s).i * pop(s).i);
			break;
		case BytecodeType::MULT_F:
			s.emplace(pop(s).f * pop(s).f);
			break;

		case BytecodeType::DIV_I: {
			auto s2 = pop(s);
			s.emplace(pop(s).i / s2.i);
			break;
		}
		case BytecodeType::DIV_F: {
			auto s2 = pop(s);
			s.emplace(pop(s).f / s2.f);
			break;
		}

		// stack values are in opposite order, so we switch signs to account for that
		case BytecodeType::LESSER_I:
			s.emplace(int64_t(pop(s).i > pop(s).i));
			break;
		case BytecodeType::LESSER_F:
			s.emplace(int64_t(pop(s).f > pop(s).f));
			break;
		case BytecodeType::LESSER_S:
			s.emplace(int64_t(pop(s).s < pop(s).s));
			break;

		case BytecodeType::GREATER_I:
			s.emplace(int64_t(pop(s).i < pop(s).i));
			break;
		case BytecodeType::GREATER_F:
			s.emplace(int64_t(pop(s).f < pop(s).f));
			break;
		case BytecodeType::GREATER_S:
			s.emplace(int64_t(pop(s).s > pop(s).s));
			break;

		case BytecodeType::LESSER_EQUALS_I:
			s.emplace((int64_t)(pop(s).i >= pop(s).i));
			break;
		case BytecodeType::LESSER_EQUALS_F:
			s.emplace((int64_t)(pop(s).f >= pop(s).f));
			break;
		case BytecodeType::LESSER_EQUALS_S:
			s.emplace((int64_t)(pop(s).s >= pop(s).s));
			break;

		case BytecodeType::GREATER_EQUALS_I:
			s.emplace((int64_t)(pop(s).i <= pop(s).i));
			break;
		case BytecodeType::GREATER_EQUALS_F:
			s.emplace((int64_t)(pop(s).f <= pop(s).f));
			break;
		case BytecodeType::GREATER_EQUALS_S:
			s.emplace((int64_t)(pop(s).s <= pop(s).s));
			break;

		case BytecodeType::EQUALS_I:
			s.emplace(int64_t(pop(s).i == pop(s).i));
			break;
		case BytecodeType::EQUALS_F:
			s.emplace(int64_t(pop(s).f == pop(s).f));
			break;
		case BytecodeType::EQUALS_S:
			s.emplace(int64_t(pop(s).s == pop(s).s));
			break;

		case BytecodeType::AND: {
			s.emplace(int64_t(pop(s).i && pop(s).i));
			break;
		}
		case BytecodeType::OR: {
			s.emplace(int64_t(pop(s).i || pop(s).i));
			break;
		}

		case BytecodeType::SUBSCRIPT:
			push_subscript(s);
			break;

		case BytecodeType::I2F:
			s.emplace(float(pop(s).i));
			break;
		case BytecodeType::F2I:
			s.emplace(int64_t(pop(s).f));
			break;

		case BytecodeType::LOAD:
			s.emplace(scope.vars[*(++it)]);
			break;

		case BytecodeType::STORE:
			scope.vars[*(++it)] = pop(s);
			break;

		case BytecodeType::SET_INDEX: {
			auto expr = pop(s);
			auto id = *(++it);
			intpr::Value* val = &scope.vars[id];
			while (!s.empty())
			{
				auto i = pop(s).i;
				val = &val->v[i];
			}
			*val = expr;
			break;
		}

		case BytecodeType::JUMP_IF_FALSE:
			if (!pop(s).i)
				std::advance(it, *(++it));
			else
				++it;
			break;

		case BytecodeType::JUMP:
			std::advance(it, *(++it));
			break;
		case BytecodeType::NJUMP:
			std::advance(it, -(*(++it)));
			break;

		case BytecodeType::RETURN:
			if (s.empty())
				return std::optional<intpr::Value>(std::nullopt);
			return pop(s);

		case BytecodeType::CALL: {
			auto id = *(++it);

			switch (id)
			{
			case 0: std::cout << (pop(s).i ? "true" : "false"); break;
			case 1: std::cout << (char)pop(s).i; break;
			case 2: std::cout << pop(s).i; break;
			case 3: std::cout << pop(s).f; break;
			case 4: std::cout << pop(s).s; break;
			case 5: {
				std::string str;
				std::getline(std::cin, str);
				s.emplace(str);
				break;
			}
			case 6: {
				s.emplace(std::string(1, char(pop(s).i)));
				break;
			}
			case 7: {
				s.emplace((int64_t)std::stoll(pop(s).s));
				break;
			}
			case 8: {
				// char is already stored as an int, so int(char) does nothing
				break;
			}
			case 9: {
				s.emplace(std::to_string(pop(s).i));
				break;
			}
			case 10: {
				s.emplace((int64_t)pop(s).s.length());
				break;
			}
			default: {
				InterpreterScope func_scope{ scope.vars };

				for (int i = 0; i < scope.funcs[id].param_ids.size(); ++i)
					func_scope.vars[InterpreterScope::funcs[id].param_ids[i]] = pop(s);

				auto rtn_value = interpret_bytecodes(func_scope, InterpreterScope::funcs[id].codes);
				if (rtn_value.has_value())
					s.push(*rtn_value);

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

void push_float(std::stack<intpr::Value>& s, bytecodes_t::const_iterator& it)
{
	int count;
	switch ((BytecodeType)(*(it++)))
	{
	case BytecodeType::FLOAT4: count = 4; break;
	case BytecodeType::FLOAT8: count = 8; break;
	default:
		throw debug::unhandled_case(*it);
	}

	float restoredFloat;
	std::memcpy(&restoredFloat, &(*it), count);

	s.emplace(restoredFloat);

	// -1 to account for the advancement in the main for loop
	std::advance(it, count - 1);
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

void push_arr(std::stack<intpr::Value>& s, bytecodes_t::const_iterator& it)
{
	bytecode_t size = *(++it);

	std::vector<intpr::Value> v;
	for (int i = 0; i < size; ++i)
		v.push_back(pop(s));

	s.emplace(v);
}

void push_subscript(std::stack<intpr::Value>& s)
{
	auto container = pop(s);
	auto index = pop(s);

	if (container.type == intpr::ValueType::ARR)
		s.emplace(container.v.at(index.i));
	else if (container.type == intpr::ValueType::PTR)
		s.emplace(container.p->v[index.i]);
	else if (container.type == intpr::ValueType::STR)
		s.emplace(int64_t(container.s.at(index.i)));
	else
		debug::unhandled_case((int)container.type);
}

intpr::Value pop(std::stack<intpr::Value>& s)
{
	assert(!s.empty());

	auto val = s.top();
	s.pop();

	return val;
}