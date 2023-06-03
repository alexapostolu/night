#include "test.hpp"

Test::Test()
{

}

Test& Test::get()
{
	static Test instance;
	return instance;
}