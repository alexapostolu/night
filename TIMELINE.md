A timeline of both past and future versions.

## Version 2 - Advanced Night

Networking...

## Version 1 - Basic Night

A complete rewrite focused on making the language dynamically typed, and adding useful syntax like array slicing, in addition to adding core features like classes and file handling.
- `v1.0` will mainly focus on making the language safe and scalable
- `v1.1` will be focused around encapsulation
- `v1.2` will be adding utilities like file and error handling
- `v1.3` networking??

### v1.2.1

Added error handling, runtime errors can be caught.

### v1.2.0

Added file handling.

### v1.1.2

Added modules, similar to Python's modules.

### v1.1.1

Added enumerations, but not like Java's weird enums, they're more like C's enums.

### v1.1.0

Added classes. However they can only contain variables and methods, no inheritance, visibility, etc. So think of them as C structs

### v1.0.3 Citizens

Types and expression as first class citizens.

### v1.0.2 Scalability

The implementation is now way more stable and scalable, allowing for faster and safer updates.

### v1.1.0 Feature Boost

Feature boost! We now have casts, pass by reference, and default parameters!

### v1.0.0

The first actual good version of Night!

This version turns Night into a dynamically typed language, with the `set` keyword used to create a variable, and the `def` keyword for functions. Also removed semicolons and made curly braces in conditionals and loops optional if there's only one statement.

There's also compile time checks to see if variables and functions exist, if functions have the right number of parameters, and expressions. This should catch a lot of hidden runtime bugs and make the code way more stable. (take that Python!)

And we're also back to using C++'s STL instead my own. And the entire parser and interpreter system is reworked. We now have an actual parser that constructs an AST and all that stuff.

## Version 0 - Beta Night

This version was me learning how to make a language :)

### v0.2.3-beta

Added compatibility with the standard library and the package manager.

### v0.2.2-beta

Functions could now return arrays and accept them as parameters.

### v0.2.1-beta

Fixed a small bug with source builds for Linux.

### v0.2.0-beta

This version got rid of many bugs and played around with the language itself.

Changed how expressions are evaluated, making it more efficient. Also created my own string and vector class (why? idk don't ask me).

### v0.1.4-beta

Added user input in the form a function.

### v0.1.3-beta

Added flexible arrays that can be reassigned to other arrays.

## v0.1.2-beta

Added a simple loop that would iterate a set amount of times.

## v0.1.1-beta

Added more support for expressions, along with assignment operators like `+=` and `*=`.

## v0.1.0-beta

The base release for `v0.1`. This version focuses on testing new features.

Turned the `print` keyword into a function.

Also made the code more modular and just better in general, especially when it came to the interpreter. And I made an actual error class.

## v0.0.0-beta

The first version of Night! (the code is so bad plz don't look at it)

It supported 5 basic types, bools, chars, ints, floats, and strings. It also supported variables, expressions, conditionals, and functions, along with a `print` keyword.

Not bad for the first version actually, considering that I didn't even know the difference between an interpreted and compiled language :/
