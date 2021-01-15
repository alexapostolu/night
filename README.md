<p align="center">
  <img src="https://github.com/DynamicSquid/night/blob/master/docs/media/night-logo-black.png"/>
</p>

# Night

Night is an interpreted dynamically typed language built around safety. It being gradually typed, combined with a robust and safe type system, makes Night a simple yet powerful language.

What makes Night different from other dynamically typed languages is its compile time checks. Whereas a language like Python does not provide type checking and many other declaration checks, Night fills this gap, eliminating hidden runtime bugs and providing safer code.

* [Getting Started](#getting-started-with-night)
* [About](#about-night)

---

### Getting Started with Night

Night comes in two parts, the interpreter (this repo) and [Dusk](https://github.com/firefish111/dusk) (the package manager). You can find easy installation and setup, as well as a tutorial, for both on the [Night website](https://night-website.dynamicsquid.repl.co/index.html).

And if you wish, you can build Night from source as well.

### Building from Source

1. Install *g++* or *clang++*

2. Clone this repository

If you would like to clone Night and Dusk (recommend), be sure to fetch it's submodule:

```
git clone --recurse-submodules https://github.com/DynamicSquid/night.git
cd night
```

Or if you want to clone Night but not Dusk, then do:

```
git clone https://github.com/DynamicSquid/night.git
cd night
```

3. Compile Night

With *g++* or *clang* (make sure it's C++17!):

```
g++ -std=c++17 -o night src/main.cpp

clang++ -std=c++17 -o night src/main.cpp
```

Or if you have GNU Make:

```
make
```

And that's it! To build Dusk from source, follow the tutorial on the [Dusk repo](https://github.com/firefish111/dusk).

### Running Night

To run Night, first navigate to the directory in which you cloned this repo. Then create a `*.night` file where you'll write your actual code. Then just pass it in as a command line argument:

```
night *.night
```

And you're done!

---

### About Night

Night is an imperative dynamically typed language with light syntax.

Here is a little sample of Night:

```py
# fibonacci sequence using recursion
def fib(num)
{
    if (num <= 1)
        return num

    return fib(num - 1) + fib(num - 2)
}

# array of two values, and appending a third
set fib_nums = [ fib(5), fib(6) ]
fib_nums = fib_nums.push(fib(7))

# printing out the values of the array
for (num : fib_nums)
    print(num + ' ')
```

Night avoids most runtime errors by checking types and declarations early on in the compile stage, something many other dynamically typed languages don't do.

```py
def add(x, y)
{
    return x + y
}

if (false)
{
    add(2, 3, 7) # error! too many parameters in function call
    a = 5 # error! variable 'a' has not been defined yet
}
```

More information regarding the syntax can be found on the [Night website](http://night.dynamicsquid.repl.co/).
