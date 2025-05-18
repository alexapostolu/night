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

	bool is_prim() const;
	bool is_str() const;
	bool is_arr() const;

	Type() = default;

	Type(std::string const& _type, int _dim = 0);
	Type(Primitive _prim, int _dim = 0);
	Type(Type const& _other);

	bool operator==(Primitive _prim) const;
	bool operator==(Type const& _type) const;
};

namespace night
{

std::string to_str(Type const& type);

}