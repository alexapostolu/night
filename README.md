# Night

Night is a bytecode interpreted language.

## Table of contents 

- [Usage](#usage)
- [Build](#build)
- [Tests](#tests)

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

## Tests

There's a testing directory for impractical code, and a sample directory for real world code, both of which come with a testing script that takes in the `night` executable.

```
night\tests> sh test_night.sh ../build/night.exe
night\samples> sh test_samples.sh ../build/night.exe
```

Expected output:

```
Running 3 tests...

programs/arrays.night Passed.
programs/basic.night Passed.
programs/functions.night Passed.

All tests passed.
```

```
Running 15 samples...

programs/ccc/2025_j3.night Passed.
programs/cs50/w1_cash.night Passed.
programs/cs50/w1_credit.night Passed.
programs/cs50/w1_mario.night Passed.
programs/cs50/w2_caesar.night Passed.
programs/cs50/w2_readability.night Passed.
programs/cs50/w2_scrabble.night Passed.
programs/cs50/w2_substitution.night Passed.
programs/leetcode/1_twosum.night Passed.
programs/leetcode/20_parantheses.night Passed.
programs/leetcode/21_mergelists.night Passed.
programs/leetcode/26_removeduplicates.night Passed.
programs/leetcode/9_palindrome.night Passed.
programs/math/collatz_conjecture.night Passed.
programs/math/fizzbuzz.night Passed.

All samples passed.
```
