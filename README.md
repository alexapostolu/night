# Night

Night is a bytecode interpreted language.

## Table of contents 

- [Usage](#usage)
- [Build](#build)
- [How it Works](#how-it-works)

---

## Usage

You can download either the Windows, Linux or Mac binaries in the [Releases section](https://github.com/alexapostolu/night/releases).

Then create a `*.night` file containing your Night code. Example programs can be found under the [`programs`](https://github.com/alexapostolu/night/tree/main/tests/programs) directory.

To run a Night file, simply type,

```
./night source.night
```

Try it out with this simple Night code!

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

The [`programs`](https://github.com/alexapostolu/night/tree/main/tests/programs) directory contains sample programs written in Night. These programs are also used in the CI pipeline as integration tests.

---

## Build

Dependenies,
* `cmake`
* `g++` or `clang`

First, clone the repo and move into that directory. Configure `cmake` and build (optionally specify a Release build with `--Release`). You should then see a `night` exectuable file in the current directory.

```sh
git clone https://github.com/alex/apostolu/night.git
cd night

mkdir build
cd build

cmake ..
cmake --build .

./night source.night
```
