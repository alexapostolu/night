# Night

<<<<<<< HEAD
An interpreted programming language that focuses on simplicity and usability. The main goal of Night is to design an intuitive and easy to learn language.
=======
An interpreted programming language that focuses on simplicity and useability. The main goal of Night is to design an intuitive and easy to learn language.
>>>>>>> 7e24f3fbd44a8500f75d6986a8702d0cb0b59496

It's still in early development, and there's a lot to get done. The goal here is to eventually be able to code Night using Night.

---

### Getting Started with Night

<<<<<<< HEAD
Night comes in two parts, the interpreter (this repo) and Dusk (the package manager). You can find easy installation and setup for both on the [Night website](https://night-website.dynamicsquid.repl.co/index.html).

You can also build 

And if you wish, you can build Night from source as well.

### Building from Source

1. Install *g++* or *clang++*
=======
Quick and easy installating and setup can be found on the [Night website](https://night-website.dynamicsquid.repl.co/html/source.html). There you'll also find instructions on how to use *dusk*, the package manager.

You can also build Night from source if you'd like.

### Building from Source

1. Install *g++* or *clang++*

2. Clone this repository
>>>>>>> 7e24f3fbd44a8500f75d6986a8702d0cb0b59496

2. Clone this repository

If you would like to clone Night but not Dusk, then do:

```
git clone https://github.com/DynamicSquid/night.git
cd night
```
<<<<<<< HEAD

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

And you're done! TO build Dusk from source, follow the tutorial on the Dusk repo

### Running Night

To run Night, first create a 
=======
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

And you're done!

To install *dusk* from source, visit the [*dusk* repo](https://github.com/firefish111/dusk).
>>>>>>> 7e24f3fbd44a8500f75d6986a8702d0cb0b59496

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
- [ ] `v5` a rewrite of Night - using Night
<<<<<<< HEAD
  - [ ] `v5.0` the base release
=======
  - [ ] `v5.0` the base release

Note that this timeline will surely change in the future.
>>>>>>> 7e24f3fbd44a8500f75d6986a8702d0cb0b59496
