# Night

An interpreted programming language that focuses on simplicity and usability. The main goal of Night is to design an intuitive and easy to learn language.

It's still in early development, and there's a lot to get done. The goal here is to eventually be able to code Night using Night.

---

### Getting Started with Night

Night comes in two parts, the interpreter (this repo) and [Dusk](https://github.com/firefish111/dusk) (the package manager). You can find easy installation and setup, as well as a tutorial, for both on the [Night website](https://night-website.dynamicsquid.repl.co/index.html).

And if you wish, you can build Night from source as well.

### Building from Source

1. Install *g++* or *clang++*

2. Clone this repository

If you would like to clone Night but not Dusk, then do:

```
git clone https://github.com/DynamicSquid/night.git
cd night
```

Or if you want to include Dusk, be sure to fetch it's submodule:

```
git clone --recurse-submodules https://github.com/DynamicSquid/night.git
cd night
```

3. Compile Night

With *g++* or *clang*:

```
g++ -o night src/night.cpp

clang++ -o night src/night.cpp
```

Or if you have GNU Make:

```
make
```

And that's it! To build Dusk from source separately, follow the tutorial on the [Dusk repo](https://github.com/firefish111/dusk).

### Running Night

To run Night, first navigate to the directory in which you cloned this repo. Then create a `*.night` file where you'll write your actual code. Then just pass it in as a command line argument:

```
night *.night
```

And you're done!

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

### Timeline

A timeline of Night, both past and future versions. Each new release, the code is scrapped and started anew (cause I have no idea what I'm doing).

- [x] `v1-beta` the first version of Night! (the code is so bad plz don't look at it)
  - [x] `v1.0-beta` the base release
- [x] `v2-beta` a testing release, many new features were tested and implemented in here
  - [x] `v2.0-beta` the base release
  - [x] `v2.1-beta` added more support for expressions
  - [x] `v2.2-beta` added a simple loop
  - [x] `v2.3-beta` added arrays
  - [x] `v2.4-beta` added user input
- [ ] `v3` the first non-beta version of Night!
  - [x] `v3.0` the base release
    - [x] `v3.0.1` bug fix for source builds
  - [x] `v3.1` functions will be able to return arrays, and accept array parameters
  - [x] `v3.2` *sqdlib*, the standard library for Night; also included is the *dusk* package manager!
  - [ ] `v3.3` bug fix for a lot of stuff
- [ ] `v4` a complete rewrite of Night, focusing on additional stability
  - [ ] `v4.0` the base release
  - [ ] `v4.1` encapsulation
    - [ ] `v4.1.1` classes, but only with variables and methods
    - [ ] `v4.1.2` enumerations
    - [ ] `v4.1.3` modules
  - [ ] `v4.2` utilities
    - [ ] `v4.2.1` file handeling
    - [ ] `v4.2.2` error handeling