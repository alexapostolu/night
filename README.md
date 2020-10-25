# Night

An interpreted programming language that focuses on simplicity and useability. The main goal of Night is to design an intuitive and easy to learn language.

It's still in early development, and there's a lot to get done.

---

### Getting Started with Night

Quick and easy installating and setup can be found on the [Night website](https://night-website.dynamicsquid.repl.co/html/source.html). There you'll also find instructions on how to use *dusk*, the package manager.

However, you can also build Night from source if you'd like.

### Building from Source

1. Install *g++* or *clang++*

2. Clone this repository

```
git clone https://github.com/DynamicSquid/night.git
cd night
```

3. Compile Night

With *g++*:

```
g++ -o night src/night.cpp
```

Or with *clang++*:

```
clang++ -o night src/night.cpp
```

Or if you have GNU Make:

```
make
```

4. Run Night

First create a new file called `*.night` where you'll write your code. Then pass it though as a command line argument:

```
./night source.night
```

And you're done! To install *dusk* from source, visit the *[dusk repo](https://github.com/firefish111/dusk)*.

---

### Timeline

As of right now, version 3 of Night has been released, and that version is actually the first non-beta release of Night. The future releases will further improve the language through various useful features such as array parameters, and better error messages, eventually working towards a standard library.

- [x] `v1-beta` the first version of Night! (the code is so bad plz don't look at it)
  - [x] `v1.0-beta` the base release
- [x] `v2-beta` a testing release, many new features were tested and implemented in here
  - [x] `v2.0-beta` the base release
  - [x] `v2.1-beta` added more support for expressions
  - [x] `v2.2-beta` added a simple loop
  - [x] `v2.3-beta` added arrays
  - [x] `v2.4-beta` added user input
- [x] `v3` the first non-beta version of Night!
  - [x] `v3.0` the base release
    - [x] `v3.0.1` bug fix for source builds
  - [x] `v3.1` functions will be able to return arrays, and accept array parameters
  - [ ] `v3.2` *sqdlib*, the standard library for Night; also included is the *dusk* package manager!
- [ ] `v4` a complete rewrite of Night, focusing on additional stability
  - [ ] `v4.0` the base release
  - [ ] `v4.1` classes, but only with variables and methods

---

### About Night

Night is strong statically typed language that mirrors the C family in many ways. As of right now, it just supports basic functionality. It has five types, booleans, characters, integers, floats, and strings. Arrays and functions are also supported. Functions can also be one of the five types, as well as a null type. Loops are also included, with a simple for and while loop.

Here is a little sample Night:

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

More information regarding the syntax can be found on the [Night website](https://night-website.dynamicsquid.repl.co/html/reference.html).