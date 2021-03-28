<p align="center">
  <img src="https://github.com/DynamicSquid/night/blob/master/docs/media/night-logo-black.png"/>
</p>

# Night

Night is an interpreted dynamically typed language built around safety.

What makes Night different from other dynamically typed languages are its compile time checks. Whereas a language like Python does not provide type checking and many other declaration checks, Night fills this gap, eliminating hidden runtime bugs and providing safer usage.

---

### Getting Started

Night comes in two parts, the interpreter (this repo) and [Dusk](https://github.com/firefish111/dusk) (the package manager). You can find easy installation and setup, as well as a tutorial, for both on the [Night website](https://github.com/DynamicSquid/night).

And if you like, you can build Night from source as well.

## Building from Source

Currently, Night is supported in Windows and Linux.

1. Install Dependancies

`g++` (version 7+)
`cmake` (version 3+)

2. Clone Repository

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

```
cmake -G "MinGW Makefiles" ../
cmake --build .
```

4. Run Night

```
night source.night
```

And you're done! View the [webiste](https://github.com/DynamicSquid/night) for more information on the `night` executable

---

### Contributing

Contributions are always welcome! Reporting bugs you find, refactoring spaghetti, and writing tutorials on the website are just a few of the ways you can contribute.