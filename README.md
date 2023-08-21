# Night

Night is a static, bytecode interpreted language with strong management on types and memory.

---

## Project Status

`v0.0.0` Under Development

**v0.0.0**
- variable initialization and assignment
- arithmetic and boolean operators, type casting
- bool, char, int, float, str, arrays
- conditionals
- for & while loops
- functions

---

## Build

**Windows Build:**

Windows executable coming soon

dependenies

`cmake` `g++`

```
cd night

cmake -G "MinGW Makefiles" .
cmake --build .

night source.night
```

**Linux Build:**

```
cd night

cmake -G "Unix Makefiles" .
cmake --build .

./night source.night
```

`source.night` is a generic path name to your source file

---

Website is hosted here: https://github.com/alexapostolu/night-web
