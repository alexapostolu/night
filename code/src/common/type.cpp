#include "common/type.hpp"
#include "common/debug.hpp"

#include <string>
#include <unordered_map>

Type::Type(std::string const& _prim_s, dim_t _dim)
	: dim(_dim)
{
	if		(_prim_s == "bool")	  prim = Primitive::BOOL;
	else if (_prim_s == "char")   prim = Primitive::CHAR;
	else if (_prim_s == "float")  prim = Primitive::FLOAT;
	else if (_prim_s == "int8")   prim = Primitive::INT8;
	else if (_prim_s == "int16")  prim = Primitive::INT16;
	else if (_prim_s == "int32")  prim = Primitive::INT32;
	else if (_prim_s == "int64")  prim = Primitive::INT64;
	else if (_prim_s == "uint8")  prim = Primitive::uINT8;
	else if (_prim_s == "uint16") prim = Primitive::uINT16;
	else if (_prim_s == "uint32") prim = Primitive::uINT32;
	else if (_prim_s == "uint64") prim = Primitive::uINT64;
	else throw debug::unhandled_case(_prim_s);
}

Type::Type(Primitive _prim, dim_t _dim)
	: prim(_prim), dim(_dim) {}

Type::Type(Type const& _other)
	: prim(_other.prim), dim(_other.dim) {}

bool Type::operator==(Primitive _prim) const
{
	return dim == 0 &&
		(prim == _prim ||
		 (prim == Primitive::INT && Type(_prim, 0).is_int()) ||
		 (_prim == Primitive::INT && is_int()));
}

static bool is_int_l(Primitive prim)
{
	return
		prim == Primitive::INT ||
		prim == Primitive::INT8 || prim == Primitive::INT16 ||
		prim == Primitive::INT32 || prim == Primitive::INT64 ||
		prim == Primitive::uINT8 || prim == Primitive::uINT16 ||
		prim == Primitive::uINT32 || prim == Primitive::uINT64;
};

bool Type::operator==(Type const& _type) const
{
	return dim == _type.dim &&
		(prim == _type.prim ||
			(prim == Primitive::INT && is_int_l(_type.prim)) ||
			(_type.prim == Primitive::INT && is_int_l(prim)));
}

bool Type::is_prim() const { return !dim; }

bool Type::is_arr() const { return dim; }

bool Type::is_str() const { return prim == Primitive::CHAR && dim == 1; }

bool Type::is_int() const
{
	return is_prim() && (
		   prim == Primitive::INT ||
		   prim == Primitive::INT8   || prim == Primitive::INT16 ||
		   prim == Primitive::INT32  || prim == Primitive::INT64 ||
		   prim == Primitive::uINT8  || prim == Primitive::uINT16 ||
		   prim == Primitive::uINT32 || prim == Primitive::uINT64);
};

std::string night::to_str(Type const& type)
{
	std::string array_string = "";

	if (type.is_arr())
		array_string = " array[" + std::to_string(type.dim) + "]";

	switch (type.prim) {
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
	default: throw debug::unhandled_case(static_cast<int>(type.prim));
	}
}
