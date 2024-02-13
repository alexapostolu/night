/*
 * Provides the Type structure used by the front end. In the front end, the
 * actual values are not important, as only the type information is needed.
 * This is used by the parser and type checking.
 */

#pragma once

#include <optional>
#include <string>

/*
 * There are two categories of types,
 *   Primitive Types
 *   Arrays
 * 
 * Primitive types include bool, char, int, and float, and are represented as
 * dimension as well.
 */
struct Type
{
	enum Primitive {
		BOOL,
		CHAR,
		INT,
		FLOAT
	} prim;

	int dim;

	bool is_str() const;
	bool is_arr() const;

	Type() = default;

	Type(std::string const& _type, int _dim = 0);
	Type(Primitive _type, int _dim = 0);

	bool operator==(Primitive _type) const;
};

/*
 * 'is_same' compares two types by their primitives and dimensions.
 * 'is_same_or_primitive' compares types by only their dimensions (treats
 *    primitive types as the same).
 * 
 * In both functions, if one or both types are nullopt, then true is returned.
 * This is because when type checking, it is assumed a nullopt type has already
 * created a minor error, so no further errors need to be generated.
 */
bool is_same(std::optional<Type> const& type1, std::optional<Type> const& type2);
bool is_same_or_primitive(std::optional<Type> const& type1, std::optional<Type> const& type2);

namespace night
{

std::string to_str(Type const& _type);

}