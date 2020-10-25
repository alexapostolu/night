#pragma once

namespace night {

template <typename T>
class array
{
public:
	array()
		: len(0), cap(3)
	{
		arr = new T[cap];
	}

	array(const array& src)
		: len(src.len), cap(src.len + 3)
	{
		arr = new T[cap];
		arr_cpy(arr, src.arr, 0, len);
	}

	array(array&& src) noexcept
		: arr(src.arr), len(src.len), cap(src.len + 3)
	{
		src.arr = nullptr;
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
		
		if (cap >= src.len)
		{
			arr_cpy(arr, src.arr, 0, src.len);
			len = src.len;

			return *this;
		}

		delete[] arr;
		
		len = src.len;
		cap = len + 3;

		arr = new T[cap];
		arr_cpy(arr, src.arr, 0, len);

		return *this;
	}

	array& operator=(array&& src) noexcept
	{
		if (this != &src)
			return *this;

		delete[] arr;

		len = src.len;
		cap = src.cap;
		arr = src.arr;

		src.arr = nullptr;

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

	void add_back(const T& val)
	{
		if (cap > len)
		{
			arr[len++] = val;
			return;
		}

		cap += 3;

		T* temp = new T[cap];
		arr_cpy(temp, arr, 0, len);
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

	void remove(int start, int end)
	{
		for (int a = 0; a < len - end; ++a)
			arr[start + a] = arr[end + a + 1];

		len -= end - start + 1;
	}

	array access(int start, int end) const
	{
		array temp;
		for (int a = start; a <= end; ++a)
			temp.add_back(arr[a]);

		return temp;
	}

	void clear()
	{
		len = 0;
	}

private:
	void arr_cpy(T* dest, T* src, int start, int end) const
	{
		for (int a = start; a < end; ++a)
			dest[a] = src[a];
	}

private:
	T* arr;
	int len, cap;
};

} // namespace night