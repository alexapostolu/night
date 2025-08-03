#include "common/type.hpp"
#include "common/debug.hpp"

#include <string>
#include <unordered_map>

bool _is_int(Primitive prim)
{
	return prim == Primitive::INT ||
		prim == Primitive::INT8 || prim == Primitive::INT16 ||
		prim == Primitive::INT32 || prim == Primitive::INT64 ||
		prim == Primitive::uINT8 || prim == Primitive::uINT16 ||
		prim == Primitive::uINT32 || prim == Primitive::uINT64;
}

static std::unordered_map<std::string, Primitive> const string_to_primitive = {
	{ "bool",	Primitive::BOOL },
	{ "char",	Primitive::CHAR },
	{ "int8",	Primitive::INT8 },
	{ "int16",	Primitive::INT16 },
	{ "int32",	Primitive::INT32 },
	{ "int64",	Primitive::INT64 },
	{ "uint8",	Primitive::uINT8 },
	{ "uint16",	Primitive::uINT16 },
	{ "uint32",	Primitive::uINT32 },
	{ "uint64",	Primitive::uINT64 },
	{ "float",	Primitive::FLOAT }
};

Type::Type() {}

Type::Type(std::string const& _type_s, dim_t _dim)
	: dim(_dim), prim(string_to_primitive.at(_type_s)) {}

Type::Type(Primitive _prim, dim_t _dim)
	: prim(_prim), dim(_dim) {}

Type::Type(Type const& _other)
	: prim(_other.prim), dim(_other.dim) {}

bool Type::operator==(Primitive _prim) const
{
	return dim == 0 &&
		(prim == _prim ||
		(prim == Primitive::INT && _is_int(_prim)) ||
		(_prim == Primitive::INT && _is_int(prim)));
}

bool Type::operator==(Type const& _type) const
{
	return dim == _type.dim &&
		(prim == _type.prim ||
		(prim == Primitive::INT && _is_int(_type.prim)) ||
		(_type.prim == Primitive::INT && _is_int(prim)));
}

bool Type::is_prim() const
{
	return !dim;
}

bool Type::is_int() const
{
	return !dim && _is_int(prim);
}

bool Type::is_arr() const
{
	return dim;
}

bool Type::is_str() const
{
	return dim == 1 && prim == Primitive::CHAR;
}

Primitive Type::get_prim() const
{
	return prim;
}

dim_t Type::get_dim() const
{
	return dim;
}

bool is_int(Primitive prim)
{
	return _is_int(prim);
}

std::string night::to_str(Type const& type)
{
	std::string array_string = "";

	if (type.is_arr())
		array_string = " array[" + std::to_string(type.get_dim()) + "]";

	switch (type.get_prim()) {
	case Primitive::BOOL:	return "bool"	+ array_string;
	case Primitive::CHAR:	return "char"	+ array_string;
	case Primitive::INT:	return "int"	+ array_string;
	case Primitive::INT8:	return "int8"	+ array_string;
	case Primitive::INT16:	return "int16"	+ array_string;
	case Primitive::INT32:	return "int32"	+ array_string;
	case Primitive::INT64:	return "int64"	+ array_string;
	case Primitive::uINT8:	return "uint8"	+ array_string;
	case Primitive::uINT16:	return "uint16"	+ array_string;
	case Primitive::uINT32:	return "uint32"	+ array_string;
	case Primitive::uINT64:	return "uint64"	+ array_string;
	case Primitive::FLOAT:	return "float"	+ array_string;
	default: throw debug::unhandled_case(static_cast<int>(type.get_prim()));
	}
}
