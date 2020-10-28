# Night

An interpreted programming language that focuses on simplicity and usability. The main goal of Night is to design an intuitive and easy to learn language.

It's still in early development, and there's a lot to get done. The goal here is to eventually be able to bootstrap Night.

---

## Getting Started with Night

Quick and easy installating and setup can be found on the [Night website](https://night-website.dynamicsquid.repl.co/index.html). There you'll also find instructions on how to use *dusk*, the package manager.

You can also build Night from source if you'd like.

## Building from Source

1. Install *g++* or *clang++*

2. Clone this repository

With *git*:

```
git clone https://github.com/DynamicSquid/night.git
```

With *gh*: the GitHub CLI:

```
gh repo clone DynamicSquid/night
```

Then `cd` into it:

```
cd night
```

3. Compile Night

With *g++*:

```
g++ -o night ./src/night.cpp
```

Or with *clang++*:

```
clang++ -o night ./src/night.cpp
```

Or if you have GNU Make:

```
make
```

4. Run Night

First create a new file called `main.night` where you'll write your code. Then pass it though as a command line argument:

```
./night main.night
```

And you're done!

To install *dusk* from source, visit the [*dusk* repo](https://github.com/firefish111/dusk).

---

### About Night

Night is strong statically typed language that mirrors the C family in many ways. As of right now, it only supports basic functionality. It has five datatypes, booleans, characters, integers, floats, and strings. Arrays and functions are also supported. Functions can also return one of the five types, as well as a null type (the equivalent of `void`, if you come from a traditional C-style language). Loops are also included, with a simple for and while loop.

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

More information regarding the syntax can be found on the [Night Website](https://night-website.dynamicsquid.repl.co/html/reference.html).

### Timeline

A timeline of Night, both past and future versions. Each new release, the code is scrapped and started afresh (cause I have no idea what I'm doing).

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
  - [x] `v3.2` *sqdlib*, the standard library for Night; also included is the *dusk* package manager!
  - [ ] `v3.3` bug fix for a lot of stuff
- [ ] `v4` a complete rewrite of Night, focusing on additional stability
  - [ ] `v4.0` the base release
  - [ ] `v4.1` encapsulation
    - [ ] `v4.1.1` structs, but only with properties and methods
    - [ ] `v4.1.2` enumerations
    - [ ] `v4.1.3` modules
  - [ ] `v4.2` utilities
    - [ ] `v4.2.1` file handling
    - [ ] `v4.2.2` error handling
- [ ] `v5` the first bootstrapped release of Night
  - [ ] `v5.0` the base release

Note that this timeline will surely change in the future.
