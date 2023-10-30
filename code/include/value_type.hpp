#pragma once

#include <deque>
#include <optional>
#include <string>

using array_dim = std::deque<std::optional<std::size_t>>;

struct ValueType
{
	enum PrimType {
		BOOL,
		CHAR,
		INT,
		FLOAT
	} type;

	// If an array, dimension of the array. If std::optional is nullopt, then array can be any size.
	array_dim dim;

	bool is_arr() const;
	bool is_str() const;

	ValueType() = default;

	ValueType(std::string const& _type, array_dim const& _dim = {});
	ValueType(PrimType _type, array_dim const& _dim = {});

	bool operator==(PrimType _type) const;
};

inline ValueType const value_type_str = ValueType(ValueType::CHAR, { std::nullopt });

bool is_same(ValueType const& vt1, ValueType const& vt2);
bool is_same_or_primitive(ValueType const& vt1, ValueType const& vt2);

namespace night
{

std::string to_str(ValueType const& vt);

}