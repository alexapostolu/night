<p align="center">
  <img src="https://github.com/DynamicSquid/night/blob/master/docs/media/night-logo-black.png"/>
</p>

[![issues](https://img.shields.io/github/issues/DynamicSquid/night)](https://github.com/DynamicSquid/night/issues)
[![pull requests](https://img.shields.io/github/issues-pr/DynamicSquid/night)](https://github.com/DynamicSquid/night/pulls)
![code size](https://img.shields.io/github/languages/code-size/DynamicSquid/night)
![total lines](https://img.shields.io/tokei/lines/github.com/DynamicSquid/night?color=176606)

# Night

Night is an interpreted dynamically typed language with a strong control on types.

Night provides compile time definition and type checking, in addition with static types. It is also planned to treat types as first class citizens, allowing the user to have more control over compile time type checking.

---

## Getting Started

Night comes in two parts: The Interpreter (this repo) and The Package Manager, [Dusk](https://github.com/firefish111/dusk).

You can download the latest stable Windows and Linux binaries for Night in the Releases section.

You can also build Night from source.

### Building from Source

1. Install Dependencies

`g++` (version 10+)<br\>
`cmake` (version 3+)

2. Clone Repository

If you would like to clone Night and Dusk (recommended), be sure to fetch it's submodule:

```
git clone --recurse-submodules https://github.com/DynamicSquid/night.git
cd night
```

If you want to clone Night but not Dusk:

```
git clone https://github.com/DynamicSquid/night.git
cd night
```

3. Compile Night

Windows:

```
cmake -G "MinGW Makefiles" .
cmake --build .
```

Linux:

```
cmake -G "Unix Makefiles" .
cmake --build .
```

4. Run Night

Create a file (preferably with a `.night` extension) where you'll write your code.
Note to replace the path in brackets with **the actual** file path.

Windows:

```
night <path\to\file>
```

Linux:

```
./night <path/to/file>
```

And you're done!

---

## Contributing

Contributions are always welcome! Reporting bugs you find, refactoring spaghetti, and writing tutorials on the website are just a few of the ways you can contribute.
