#include "../include/token.hpp"

bool Conditional::is_else() const
{
	return condition == nullptr;
}