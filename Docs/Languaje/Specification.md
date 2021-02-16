# Rift Languaje specification

## Structs
**Structs** represent a group of variables together in memory.
They are passed-by-value by default, meaning they get copied.

If decided (manually) structs can be passed by reference.

Structs can have functions, but they are prohibited from modifying external memory (not part of the struct), and they are never virtual.

Structs's memory is inlined in the type that contains it. (This also means that an struct can't contain itself as a nested struct)


## Classes
**Classes** are similar to structs, with the difference that they get passed-by-reference or pointer by default, and their functions have no limitations.
Class functions can be marked as virtual.


## Function Libraries
**Function Libraries** only contain static functions and don't have any representation in memory.
