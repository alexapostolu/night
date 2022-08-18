<p align="center">
  <img src="https://github.com/DynamicSquid/night/blob/master/docs/media/night-logo-black.png"/>
</p>

# Night

Under development.

Night is an interpreted dynamically typed language with a strong control on types.

---

## Getting Started

Right now you can only build Night from source.

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
