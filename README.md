# Night

**Table of Contents**

- (Overview)[https://github.com/AntimatterReactor/Night/blob/master/README.md#overview]
- (Progress)[https://github.com/AntimatterReactor/Night/blob/master/README.md#progress]
- (Syntax)[https://github.com/AntimatterReactor/Night/blob/master/README.md#syntax]
  - (Comparison)[https://github.com/AntimatterReactor/Night/blob/master/README.md#comparison]
  - (Difference)[https://github.com/AntimatterReactor/Night/blob/master/README.md#difference]
- (Further Reading)[https://github.com/AntimatterReactor/Night/blob/master/README.md#further-reading]
- (Notable Links)[https://github.com/AntimatterReactor/Night/blob/master/README.md#notable-links]

## Overview

An interpreted programming language that combines the simplicity of Python, with the type concepts of the C family.

It's still in early development, and there's a lot to get done.

## Progress

As of right now, version 3 of Night has been released, and that version is actually the first non-beta release of Night. The future releases will further improve the language through various useful features such as array parameters, and better error messages, eventually working towards a standard library.

Below is a list of the upcoming features:

- `v3.1` arrays with functions
  - this release will be the last major step towards the making of the standard library
  - functions will be able to return arrays
  - functions will also be able to have array parameters
- `v3.2` better error messages
  - like, a lot better
- `v3.3` [sqdlib](https://github.com/DynamicSquid/sqdlib)
  - the standard library for Night, it stands for "squid library" :)
  - a package manager might also be included...
    - you can check the issues and pull requests for more info on this

## Syntax

Syntax was incredibly similar (or even the same) as c++

### Comparison

Cross Comparison of Fibonacci Sequence

Night:

```cpp
// classic fibonacci sequence using recursion
int fib(int num)
{
    if (num <= 1)
    {
        return num;
    }

    return fib(num - 1) + fib(num - 2);
}

// array of 3 values, 2 of which have been initialized
int[3] fib_nums = [ fib(5), fib(6) ];
fib_nums[2] = fib(7);

// printing out the values of the array
int a = 0;
loop for (3)
{
    print(fib_nums[a] + " ");
    a += 1;
}
```

C++:

```c++
#include <iostream>
// classic fibonacci sequence using recursion
int fib(int num)
{
  if (num <= 1)
  {
      return num;
  }

  return (fib(num - 1) + fib(num - 2));
}

int main()
{
  // array of 3 values, 2 of which have been initialized
  int fib_nums[3] = { fib(5), fib(6) };
  fib_nums[2] = fib(7);
  
  // printing out the values of the array
  int a = 0;
  for (size_t i; i < 3; ++i)
  {
      print(fib_nums[a] + " ");
      a += 1;
  }
}
```

Python:

```python
# classic fibonacci sequence using recursion
def fib(num):

  if num <= 1:
    
    return num
    

  return (fib(num - 1) + fib(num - 2))s

fib_nums = [ fib(5), fib(6) ]
fib_nums.append(fib(7))

# printing out the values of the array
print(fib_nums)
```

### Difference

Night's most notable difference to C is the datatypes names:

int -> int

char -> syb ("single byte")

float -> dec ("decimal")

char* (std::string (cpp)) -> str

Also, lack of `int main()`, array declaration with `[]` instead of `{}`, and loops
with weird syntax; `loop for (5)` instead of `for (size_t i; i < 5; ++i)`.

## Further Reading

Wiki:

Website: (https://night-website.dynamicsquid.repl.co/)

## Notable Links

Website: (https://night-website.dynamicsquid.repl.co/)

Packet Manager: (dusk)[https://github.com/firefish111/dusk]

Standard Library: (sqdlib)[https://github.com/DynamicSquid/sqdlib]

Pseudo Compiler: (NightShade)[https://github.com/AntimatterReactor/NightShade]
