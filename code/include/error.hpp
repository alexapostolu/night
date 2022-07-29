#pragma once

namespace night {

class error
{
public:
	error(error const&) = delete;

public:
	static error& get();

public:
	void operator=(error const&) = delete;

private:
	error();

public:
	bool debug_flag;
};

}