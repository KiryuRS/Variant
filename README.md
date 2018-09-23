# Variant - Typeless Data Structure
This documentation describes about a typeless Variant that provides an alternative to C++17 std::variant. The idea is inspired from a problem of passing in typeless objects through a messaging system. Even with the implementation of std::variant, users would still have to specify the supported types in that data structure - hence the incovenience for users.

As there are variations of Variants in C++, most of the codes would require minimum hard-coding of supported data types (i.e. multiple switch cases or similar to retrieve all possible types). Despite having some format to follow, the Variant class allows any supported UDT types, as long as they adhere to the criteria and adding the type into the list of supported Variant types.


## Future Implementations
* Allowing template type deduction for Functors when using visit
* Support for C++11


## Stable Versions
* Compilers with C++17 fully supported

### Supported C++ Versions
* GNU with at least C++14
* CL with at least Version 19.14


# Known Issues
* Does not work when you pass the type in as const char* (i.e. c-style string) when compiled with C++14 as _if constexpr_ is not available in C++14.
* C++14 has yet to be fully tested for potential bugs.
