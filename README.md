<p align="center">
  <img src="https://github.com/DynamicSquid/night/blob/master/docs/media/night-logo-black.png"/>
</p>

# Night

Night is an interpreted dynamically typed language with a strong control on types.

Night provides compile time definition and type checking, in addition with static types. It is also planned to treat types as first class citizens, allowing the user to have more control over compile time type checking.

---

## Getting Started

Night comes in two parts, the interpreter (this repo) and [Dusk](https://github.com/firefish111/dusk) (the package manager).

You can download the latest stable Windows and Linux binaries for Night in the Releases section.

You can also build Night from source.

### Building from Source

1. Install Dependancies

`g++` (version 10+)<br>
`cmake` (version 3+)

2. Clone Repository

If you would like to clone Night and Dusk (recommend), be sure to fetch it's submodule:

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

Create a file called `source.night` where you'll write your code.

Windows:

```
night source.night
```

Linux:

```
./night source.night
```

And you're done!
