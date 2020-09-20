#pragma once

namespace night {

class string
{
public:
	string()
	{
		len = 0, cap = 21;
		str = str_alc(cap);
	}

	string(char c)
	{
		len = 1, cap = 22;

		str = str_alc(cap);
		str[0] = c;
	}

	string(const char* src)
	{
		len = str_len(src);
		cap = len + 21;

		str = str_alc(cap);
		str_cpy(str, src);
	}

	string(const string& src)
	{
		len = src.len;
		cap = len + 21;

		str = str_alc(cap);
		str_cpy(str, src.str);
	}

	string(string&& src) noexcept
	{
		len = src.len;
		cap = src.cap;
		str = src.str;

		src.str = nullptr;
	}

	~string()
	{
		delete[] str;
	}

public:
	string& operator=(const char* src)
	{
		delete[] str;

		len = str_len(src);
		cap = len + 21;

		str = str_alc(cap);
		str_cpy(str, src);

		return *this;
	}

	string& operator=(const string& src)
	{
		if (this == &src)
			return *this;

		delete[] str;

		len = src.len;
		cap = len + 21;

		str = str_alc(cap);
		str_cpy(str, src.str);

		return *this;
	}

	string& operator=(string&& src) noexcept
	{
		if (this == &src)
			return *this;

		delete[] str;

		len = src.len;
		cap = src.cap;
		str = src.str;

		src.str = nullptr;

		return *this;
	}

	night::string operator+(char c) const
	{
		char* temp = str_alc(len + 22);
		str_cpy(temp, str);
		temp[len] = c;

		return temp;
	}

	night::string operator+(const string& src) const
	{
		char* temp = str_alc(len + src.len + 21);
		str_cpy(temp, str);
		str_cpy(temp, src.str);

		return temp;
	}

	void operator+=(char c)
	{
		if (cap > len + 1)
		{
			str[len] = c;
			len++;

			return;
		}

		cap += 22;

		char* temp = str_alc(cap);
		str_cpy(temp, str);
		temp[len] = c;

		delete[] str;

		str = temp;
		len++;
	}

	void operator+=(const char* src)
	{
		if (cap > len + str_len(src))
		{
			str_cpy(str, src);
			len += str_len(src);

			return;
		}

		cap += str_len(src) + 21;

		char* temp = str_alc(cap);
		str_cpy(temp, str);
		str_cpy(temp, src);

		delete[] str;

		str = temp;
		len += str_len(src);
	}

	void operator+=(const string& src)
	{
		if (cap > len + src.len)
		{
			str_cpy(str, src.str);
			len += src.len;

			return;
		}

		cap += src.len + 21;

		char* temp = str_alc(cap);
		str_cpy(temp, str);
		str_cpy(temp, src.str);

		delete[] str;

		str = temp;
		len += src.len;
	}

	bool operator==(const char* src) const
	{
		return str_cmp(src, str_len(src));
	}

	bool operator==(const string& src) const
	{
		return str_cmp(src.str, src.len);
	}

	bool operator!=(const char* src) const
	{
		return !str_cmp(src, str_len(src));
	}

	bool operator!=(const string& src) const
	{
		return !str_cmp(src.str, src.len);
	}

	char operator[](int index) const
	{
		return str[index];
	}

	char& operator[](int index)
	{
		return str[index];
	}

public:
	int length() const
	{
		return len;
	}

	const char* cstr() const
	{
		return str;
	}

	void remove(int index)
	{
		char* temp = str_alc(cap);

		for (int a = 0; a < index; ++a)
			temp[a] = str[a];

		for (int a = index + 1; a < len; ++a)
			temp[a - 1] = str[a];

		delete[] str;

		str = temp;
		len--;
	}

private:
	int str_len(const char* src) const
	{
		int length = 0;
		for (; src[length] != '\0'; ++length);

		return length;
	}

	bool str_cmp(const char* cmp_str, int cmp_len) const
	{
		if (len != cmp_len)
			return false;

		for (int a = 0; a < len; ++a)
		{
			if (str[a] != cmp_str[a])
				return false;
		}

		return true;
	}

	void str_cpy(char* dest, const char* src) const
	{
		int start = 0;
		for (; dest[start] != '\0'; ++start);

		for (int a = 0; src[a] != '\0'; ++a)
			dest[start + a] = src[a];
	}

	char* str_alc(int alloc_cap) const
	{
		char* temp = new char[alloc_cap]{ '\0' };
		return temp;
	}

private:
	char* str;
	int len, cap;
};

} // namespace night

#if defined(__linux__)
	night::string operator""_s(const char* str, unsigned long len)
	{
		return night::string(str);
	}
#elif defined(_WIN32)
	night::string operator""_s(const char* str, unsigned int len)
	{
		return night::string(str);
	}
#endif