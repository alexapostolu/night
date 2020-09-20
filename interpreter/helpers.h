#pragma once

namespace night {

int pow(int base, int exp)
{
	int set = base;
	base = 1;
	for (int a = 0; a < exp; ++a)
		base *= set;

	return base;
}

night::string ttos(const TokenType& type)
{
	if (type == TokenType::BIT_VALUE || type == TokenType::BIT_TYPE)
		return "bit";
	if (type == TokenType::SYB_VALUE || type == TokenType::SYB_TYPE)
		return "syb";
	if (type == TokenType::INT_VALUE || type == TokenType::INT_TYPE)
		return "int";
	if (type == TokenType::DEC_VALUE || type == TokenType::DEC_TYPE)
		return "dec";
	if (type == TokenType::STR_VALUE || type == TokenType::STR_TYPE)
		return "str";
}

TokenType ttov(const TokenType& type)
{
	switch (type)
	{
	case TokenType::BIT_TYPE:
		return TokenType::BIT_VALUE;
	case TokenType::SYB_TYPE:
		return TokenType::SYB_VALUE;
	case TokenType::INT_TYPE:
		return TokenType::INT_VALUE;
	case TokenType::DEC_TYPE:
		return TokenType::DEC_VALUE;
	case TokenType::STR_TYPE:
		return TokenType::STR_VALUE;
	}
}

float stof(const night::string& val)
{
	float number = 0;
	int decimalCount = 0, decimalReset = 0;
	for (int a = val.length() - 1; a >= 0; --a)
	{
		if ((val[a] - '0' < 0 || val[a] - '0' > 9) && val[a] != '.')
			throw 1; // fix
		if (val[a] == '.' && ++decimalCount > 1)
			throw 1;

		if (val[a] == '.')
		{
			decimalReset = val.length() - a;
			number /= pow(10, decimalReset - 1);

			continue;
		}
		
		number += (val[a] - '0') * pow(10, val.length() - a - 1 - decimalReset);
	}

	return number;
}

int stoi(const night::string& val)
{
	return (int)stof(val);
}

night::string ftos(float val)
{
	night::string str;

	int num = (int)val;
	val -= num;

	do {
		str = night::string(num % 10 + '0') + str;
		num /= 10;
	} while (num > 0);

	if (val == 0)
		return str;

	str += '.';

	night::string dec;

	do {
		int ones = (int)(val * 10);
		val *= 10;
		dec += night::string((int)val + '0');
		val -= ones;
	} while (val > 0);

	return str + dec;
}

night::string itos(int val)
{
	night::string str;

	int num = (int)val;
	val -= num;

	do {
		str = night::string(num % 10 + '0') + str;
		num /= 10;
	} while (num > 0);

	return str;
}

} // namespace night

template <typename T>
T* GetObject(night::array<T>& container, const Token& object)
{
	for (int a = 0; a < container.length(); ++a)
	{
		if (object.value == container[a].name)
			return &container[a];
	}

	return nullptr;
}