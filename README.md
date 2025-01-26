# Night

Night is a bytecode interpreted language.

## Table of contents 

- [Usage](#usage)
- [Build](#build)
- [How it Works](#how-it-works)

---

## Usage

You can download either the Windows or Linux binary file in the [Releases section](https://github.com/alexapostolu/night/releases).

Then create a `.night` file containing your Night code. Example programs can be found under the [`programs`](https://github.com/alexapostolu/night/tree/main/tests/programs) directory.

To run a Night file, simply type,

```
night source.night
```

Try it out now with this simple Night code!

###### *source.night*
```py
# recursive fib function
def fib(n int) int
{
    if (n == 0 || n == 1)
        return n;

    return fib(n - 1) + fib(n - 2);
}

print("Enter a number: ");
n int = int(input());

for (i int = 0; i < n; i += 1)
    print(str(fib(i)) + " ");
```

The [`programs`](https://github.com/alexapostolu/night/tree/main/tests/programs) directory contain simple programs written in Night, including some Harvard's [CS50](https://cs50.harvard.edu/college/2023/spring/) programs and some common Math programs. These programs are also used in the CI pipeline as integration tests.

---

## Build

Dependenies, `cmake` `g++`

First, clone the repo and move into that directory. Generate MinGW or Unix Makefiles using `cmake`, and then build (optionally specify a Release build with `--Release`). You should then see a `night` exectuable file in the current directory.

**Windows Build:**

```sh
git clone https://github.com/alex/apostolu/night.git
cd night

mkdir build
cd build

cmake -G "MinGW Makefiles" ..
cmake --build .

night source.night
```

**Linux Build:**

```sh
cd night

mkdir build
cd build

cmake -G "Unix Makefiles" ..
cmake --build .

./night source.night
```
