<p align="center">
  <img src="https://github.com/DynamicSquid/night/blob/master/docs/media/night-logo-black.png"/>
</p>

# Night

Night is an interpreted dynamically typed language with a strong control on types.

Night provides compile time definition and type checking, in addition with static types. It can also treat types as first class citizens, allowing the user to have more control over compile time type checking.

---

## Current Progress

Stable version still in development, along with the website.

---

## Getting Started

Night comes in two parts, the interpreter (this repo) and [Dusk](https://github.com/firefish111/dusk) (the package manager).

Currently, Night only supports a source build. Binaries for Windows and Linux are comming soon :)

### Building from Source

The [latest stable commit](https://github.com/DynamicSquid/night/tree/8079fa9e05499b97d60b8777a6aeb733c23dffb3).

1. Install Dependancies

`g++` (version 8+)<br>
`cmake` (version 3+)

2. Clone Repository

If you would like to clone Night and Dusk (recommend), be sure to fetch it's submodule:

```
git clone --recurse-submodules https://github.com/DynamicSquid/night.git
cd night
```

Or if you want to clone Night but not Dusk:

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

Windows:

```
night source.night
```

Linux:

```
./night source.night
```

And you're done!

---

## Contributing

Contributions are always welcome! Reporting bugs you find, refactoring spaghetti, and writing tutorials on the website are just a few of the ways you can contribute.
