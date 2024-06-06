
# pi2mcrl2

## Build Instructions

The project uses [CMake](https://cmake.org/) to generate build files, so simply use the following commands:

```sh
cmake .
cmake --build .
```

If you run the tool, make sure to run it from this directory as the working directory (with the `./mcrl2` directory
relative to it). The location of the executable should be `./Debug/pi2mcrl2`.

## Grammar for the Pi Calculus Language

```rust
Module ::=
    ProcDecl*

ProcDecl ::=
    "agent" Identifier ("(" ArgList? ")")? "=" Proc ";"

ArgList ::=
    Identifier |
    Identifier "," |
    Identifier "," ArgList

Proc ::=
    ParallelProc |
    Proc "+" ParallelProc

ParallelProc ::=
    UnaryProc |
    ParallelProc "|" UnaryProc

UnaryProc ::=
    "tau" "." UnaryProc |
    Identifier "(" Identifier ")" "." UnaryProc |
    Identifier "'" Identifier "." UnaryProc |
    "[" Identifier "=" Identifier "]" UnaryProc |
    "(" "^" Identifier ")" UnaryProc |
    Identifier ("(" ArgList? ")")? |
    "0" |
    "(" Proc ")"
```

The initial process (if any) should be called `Main`. You may give `Main` an arbitrary number of parameters, which will
denote distinct unbound names.

## Links and References

["A calculus of mobile processes, I"](https://www.sciencedirect.com/science/article/pii/0890540192900084)

["A calculus of mobile processes, II"](https://www.sciencedirect.com/science/article/pii/0890540192900095)

Robin Milner: Communicating and Mobile Systems: the pi-calculus. Cambridge University Press 1999, ISBN 0 521 65869 1.

Davide Sangiorgi, David Walker: The Pi-Calculus - a theory of mobile processes. Cambridge University Press 2001, ISBN 978-0-521-78177-0, pp. I-XII, 1-580

https://github.com/markuzzz/SysML-to-mCRL2/blob/main/semantics_spec.mcrl2
