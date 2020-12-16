<p align="center">
  <img src="https://github.com/DynamicSquid/night/blob/master/docs/media/night-logo-black.png"/>
</p>

# Night

An interpreted programming language that focuses on simplicity and usability. The main goal of Night is to design an intuitive and easy to learn language.

* [Getting Started](#Getting Started with Night)

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

With *g++* or *clang*:

```
g++ -o night src/night.cpp

clang++ -o night src/night.cpp
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

Here is a little sample Night:

```py
# classic fibonacci sequence using recursion
def fib(num)
{
    if (num <= 1)
        return num

    return fib(num - 1) + fib(num - 2)
}

# array of 3 values, 2 of which have been initialized
set fib_nums = 3[ fib(5), fib(6) ]
fib_nums[2] = fib(7)

# printing out the values of the array
for (num : fib_nums)
    print(num + " ")
```

More information regarding the syntax can be found on the [Night website](https://night-website.dynamicsquid.repl.co/html/reference.html).
