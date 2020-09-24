# Night

An interpreted programming language that combines the simplicity of Python, with the type concepts of the C family.

It's still in early development, and there's a lot to get done.

You can read more about Night on its [website](https://night-website.dynamicsquid.repl.co/). There you'll find a tutorial, as well as instructions on how to build this from source.

---

**Night Overview**

Night is strong statically typed language that mirrors the C family in many ways. As of right now, it just supports basic functionality. It has five basic types, booleans, characters, integers, floats, and strings. Arrays and functions are also supported, however, functions currently cannot have array parameters, or return arrays (but that will soon change). Functions can also be one of the five types, as well as a `null` type.

Here is a little sample of my language:

```cpp
// classic ffibonacci sequence using recursion
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

More information regarding the syntax can be found on the website.

---

**Progress Updates**

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