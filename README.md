# Night

Night is a static, bytecode interpreted language with strong management on types and memory.

## Table of contents 

- [Project Status](#project-status)
- [Usage](#usage)
- [Build](#build)
- [How it Works](#how-it-works)

---

## Project Status

`v0.0.0` Under Development

**v0.0.0**
- variable initialization and assignment
- arithmetic and boolean operators, type casting
- bool, char, int, float, str, arrays (including multi-dimensional arrays)
- conditionals
- for & while loops
- functions

I have already planned future functionality, including pointers, OOP, modules, and a standard library.

## Usage

You will need a Night binary file, and a source file with Night code. To run the source file, simply type

```
night source.night
```

Binary files can be found under releases, or compiled manually as shown in the [Build](#build) section.

`tests/programs` contain simple example programs written in Night to solve various online computer science problem sets, including [`cs50`](https://cs50.harvard.edu/college/2023/spring/). These programs are also used for functional testing.

---

## Build

**Windows Build:**

Windows executable coming soon

dependenies

`cmake` `g++`

First, clone the repo and move into that directory. Generate MinGW Makefiles using `cmake`, and then build. You should then see a `night.exe` file in the current directory.

```
git clone https://github.com/alex/apostolu/night.git
cd night

cmake -G "MinGW Makefiles" .
cmake --build .

night source.night
```

**Linux Build:**

First, clone the repo and move into that directory. Generate Unix Makefiles using `cmake`, and then build. You should then see a `night` binary in the current directory.

```
cd night

cmake -G "Unix Makefiles" .
cmake --build .

./night source.night
```

`source.night` is a generic path name to your source file

---

Website is hosted here: https://github.com/alexapostolu/night-web
