#pragma once

#include <optional>
#include <string>

struct ValueType
{
	enum PrimType {
		BOOL,
		CHAR,
		INT,
		FLOAT
	} type;

	int dim;

	bool is_str() const;
	bool is_arr() const;

	ValueType() = default;

	ValueType(std::string const& _type, int _dim = 0);
	ValueType(PrimType _type, int _dim = 0);

	bool operator==(PrimType _type) const;
};

bool is_same(std::optional<ValueType> const& vt1, std::optional<ValueType> const& vt2);
bool is_same_or_primitive(std::optional<ValueType> const& vt1, std::optional<ValueType> const& vt2);

namespace night
{

std::string to_str(ValueType const& vt);

}