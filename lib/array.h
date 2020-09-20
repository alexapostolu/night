#pragma once

namespace night {

template <typename T>
class array
{
public:
	array()
	{
		len = 0, cap = 3;
		arr = new T[3];
	}

	array(const array& src)
	{
		len = src.len;
		cap = len + 3;

		arr = new T[cap];
		for (int a = 0; a < src.len; ++a)
			arr[a] = src.arr[a];
	}

	~array()
	{
		delete[] arr;
	}

public:
	array& operator=(const array& src)
	{
		if (this == &src)
			return *this;
		
		delete[] arr;
		
		len = src.len;
		cap = len + 3;

		arr = new T[cap];
		for (int a = 0; a < len; ++a)
			arr[a] = src.arr[a];

		return *this;
	}

	T operator[](int index) const
	{
		return arr[index];
	}

	T& operator[](int index)
	{
		return arr[index];
	}

public:
	int length() const
	{
		return len;
	}

	T back() const
	{
		return arr[len - 1];
	}

	T& back()
	{
		return arr[len - 1];
	}

	void push_back(const T& val)
	{
		if (cap > len)
		{
			arr[len] = val;
			len++;

			return;
		}

		cap += 4;

		T* temp = new T[cap];
		for (int a = 0; a < len; ++a)
			temp[a] = arr[a];

		temp[len] = val;

		delete[] arr;

		arr = temp;
		len++;
	}

	void remove(int index)
	{
		for (int a = index; a < len - 1; ++a)
			arr[a] = arr[a + 1];

		len--;
	}

	array access(int begin, int end) const
	{
		array temp;
		for (int a = begin; a <= end; ++a)
			temp.push_back(arr[a]);

		return temp;
	}

	void clear()
	{
		len = 0;
	}

private:
	T* arr;
	int len, cap;
};

} // namespace night