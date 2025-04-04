# C* Compiler

The C* compiler is a C-like language which attempts to modernize the C language by improving upon the preprocessor, removing legacy features, and sprinkling in a few concepts from C++. C*'s biggest features are its new preprocessor and the inclusion of a module (and header) system.

![Top language badge](https://img.shields.io/github/languages/top/Pratixx/csrcompiler?style=plastic&label=C)
![Lines of code badge](https://tokei.rs/b1/github/Pratixx/csrcompiler)

## Roadmap
- [X] Lexer
  - [X] Literals
  - [X] Keywords
  - [X] Punctuators
  - [X] Operators
- [ ] Parser
    - [X] Function and variable recognition
    - [ ] Loops
    - [ ] Conditionals
    - [ ] Expressions
    - [ ] Statements
- [ ] IR generation
- [ ] Obj generation

# Features
## New Preprocessor

The C preprocessor is by no means pretty and can be rather restrictive in certain scenarios. It has been given more flexibility, so much to the point it could be considered too powerful. It's up to the programmer to use it properly.

## Module & Header System

The inclusion of a module (and header) system will improve symbol tables internally and allow more flexibility for the programmer by not having to have both a source and a header file open. This of course requires integration of the `import` and `export` keywords, alongside `module`. Additionally, C* contains a transpiler, which turns valid C code and all C standard library functions into a valid C* module, which will make migration and usage of C code in your codebase much easier. These are differentiated from modules with the `header` keyword. This is what sets C* apart from other languages attempting to replace C.

## Legacy Keyword Elimination & New Keyword Addition

Legacy keywords such as `inline` and `register` have been removed due to modern compiler optimizations making them redundant. In addition, many keywords have been replaced by their modern counterparts, such as `auto` now providing type inference and `typedef` being entirely replaced with `using`. Namespaces have also been added solely for the purpose of better encapsulation, used through the `namespace` keyword.
