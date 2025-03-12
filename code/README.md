# Overview of the Night Executable

This guide details an overview of the entire Night implementation from start to finish. This guide does not cover

The Night executable comprises of two stages, the Preprocessor and the Implementation,

Source Code --> **Night Executable** --> Result
Source Code --> **Preprocessor** --> **Implementation** --> Result

## Preprocessor

Related Files,

`main.cpp`

`parse_args.hpp` & `parse_args.cpp`

The Preprocessor mainly handles command line arguments passed into the Night executable.

## Implementation

Preprocessor --> **Lexer** --> **Parser** --> **Interpreter** --> Result

Each of the stages are represented as a class or a set of functions. The constructor or main function for each stage converts the output from the previous stage to input for its own stage.
