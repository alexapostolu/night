# Night

An interpreted programming language that combines the simplicity of Python, with the type concepts of the C family. The main goal of Night is to design an intuitive and easy to learn language.

It's still in early development, and there's a lot to get done.

---

### Source Build

Currently, only a Windows and a Linux source build are supported.

Windows system requirements:
- GCC and a working console

Linux system requirements:
- build-essential / base-devel

You'll need to install the *GNU Compiler Collection* (GCC) to get access to the *g++* compiler, and once you have that, clone this repo, create a new file called `*.night` where you'll write your code, and you're ready to go! 
Just pass in the `*.night` file as a command line argument when you run the code.

**Windows**

```
g++ -o <exe file> <path to night.cpp file>
<exe file> *.night

for example:

g++ -o night src/night.cpp
night source.night
```

**Linux**

You can foolow the steps above, or if you have GNU Make, you can simply use that and it'll create a `night` executable.

```
make
./night source.night
```

If there's an error with the source build, or you're having trouble, feel free to leave an issue!

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
  - [ ] `v3.1` functions will be able to return arrays, and accept array parameters
  - [ ] `v3.2` *sqdlib*, the standard library for Night; also included is the *dusk* package manager!

---

### About Night

Night is strong statically typed language that mirrors the C family in many ways. As of right now, it just supports basic functionality. It has five types, booleans, characters, integers, floats, and strings. Arrays and functions are also supported. Functions can also be one of the five types, as well as a null type. Loops are also included, with a simple for and while loop.

Here is a little sample of my language:

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

More information regarding the syntax can be found on the website.

---

### Other

Here's a couple other projects related to Night, all under development and will be integrated into or used with Night sometime in the future. Definitely worth checking these out!

**sqdlib**

*[sqdlib](https://github.com/DynamicSquid/sqdlib)* (squid library) is the standard library for Night. It's coming shortly after `v3.1` and will feature some handy functions. More information about that can be found on the repo.

**dusk**

*[dusk](https://github.com/firefish111/dusk)* is the package manager for Night. It's primarily for *sqdlib*, but user built libraries could also be added later. Huge thanks to [@firefish111](https://github.com/firefish111) for making it!

**night shade**

*[night shade](https://github.com/AntimatterReactor/NightShade)* is a neat little compiler for Night. Made by [@AntimatterReactor](https://github.com/AntimatterReactor), it compiles Night down to C++ rather well, however it's not fully implemented yet.

If you have an interesting idea for Night, or would like to contribute to one of these projects, don't hesitate to leave a pull request!