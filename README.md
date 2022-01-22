# Variant - Typeless Data Structure
This documentation is about Typeless Variant that is an extension of C++17 std::any, along with similar concepts from C++17 std::variant. The idea is inspired from a problem of passing in typeless objects through a messaging system. Even with the implementation of std::variant or std::any, users would still have to specify the supported types in that data structure - hence the incovenience for users.

As there are variations of Variants in C++, most of the codes would require minimum hard-coding of supported data types (i.e. multiple switch cases or similar to retrieve all possible types). Despite having some format to follow, the Variant class allows any supported UDT types, as long as they adhere to the criteria and adding the type into the list of supported Variant types.

# Latest Build Status
[![Build Status](https://travis-ci.com/KiryuRS/Variant.svg?branch=master)](https://travis-ci.com/KiryuRS/Variant)

## Stable Versions
* Compilers with C++17 fully supported

### Supported C++ Versions
* GNU with at least C++14
* MSVC with at least C++17
* C++17 Build

# Documentation
## Sanity Check
1. Ensure that the type that you want Variant to incorporate is included under 'VARIANT_TYPES' in variant_impl.h
3. All custom types that Variant supports should consist of a Default Constructor and operator== overloading function
***
## Use Case Examples
```C++
ki::Variant v1{ 5 };				// v1's type is int and value of 5
int i{ v1 };                                    // i contains 5
v1.set_value(std::string{ "Hello World" });	// v1's type is std::string and value of "Hello World"
float f = 3.14159f;
v1 = f;						// v1's type is float and value of 3.14159f
v1 = 1.2345f;					// v1's type REMAINS the same but value changed to 1.2345f
ki::Variant v2{ v1 };				// v2's type is float and value of 1.2345f
v1 == v2;					// expression is true	[ Prioritized on type followed by value ]
v1 != v2;					// expression is false
v1.type().name();				// float [ Variant::Type() returns std::type_index ]
ki::Variant v3;					// v3 has no type and no value
std::vector<ki::Variant> vec_var;			// std::vector of variants
vec_var.emplace_back(std::string{ });
vec_var.emplace_back(float{});
vec_var.emplace_back(long long{});
```
***
