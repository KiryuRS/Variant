# Variant - Typeless Data Structure
This documentation describes about a typeless Variant that provides an alternative to C++17 std::variant. The idea is inspired from a problem of passing in typeless objects through a messaging system. Even with the implementation of std::variant, users would still have to specify the supported types in that data structure - hence the incovenience for users.

As there are variations of Variants in C++, most of the codes would require minimum hard-coding of supported data types (i.e. multiple switch cases or similar to retrieve all possible types). Despite having some format to follow, the Variant class allows any supported UDT types, as long as they adhere to the criteria and adding the type into the list of supported Variant types.

# Latest Build Status
[![Build Status](https://travis-ci.com/KiryuRS/Variant.svg?branch=master)](https://travis-ci.com/KiryuRS/Variant)

## Future Implementations
* ~~Allowing template type deduction for Functors when using visit~~
* Support for C++11


## Stable Versions
* Compilers with C++14 fully supported

### Supported C++ Versions
* GNU with at least C++14
* CL with at least Version 19.14
* C++17 Build

# Documentation
## Sanity Check
1. Ensure that the type that you want Variant to incorporate is included under 'VARIANTTYPES' in Common.h
2. ~~Add the relevant switch cases in the Copy Constructor~~
3. All custom types that Variant supports should consist of a Default Constructor and operator== overloading function
***
## Use Case Examples
```C++
Variant v1{ 5 };				// v1's type is int and value of 5
int i{ v1 };                                    // i contains 5
v1.set_value(std::string{ "Hello World" });	// v1's type is std::string and value of "Hello World"
float f = 3.14159f;
v1 = f;						// v1's type is float and value of 3.14159f
v1 = 1.2345f;					// v1's type REMAINS the same but value changed to 1.2345f
Variant v2{ v1 };				// v2's type is float and value of 1.2345f
v1 == v2;					// expression is true	[ Prioritized on type followed by value ]
v1 != v2;					// expression is false
v1.type().name();				// float [ Variant::Type() returns std::type_index ]
Variant v3;					// v3 has no type and no value
std::vector<Variant> vec_var;			// std::vector of variants
vec_var.emplace_back(std::string{ });
vec_var.emplace_back(float{});
vec_var.emplace_back(long long{});
```
***
## Documentation
1. ALL Built-in types by default can be supported by Variant
2. UDT that wants to be supported by Variant have to include the following functions:
	- Default Constructor
	- operator==(...) overloading
	- ~~friend std::ostream& operator<<(std::ostream&, ...)~~
3. Variant class will not have a generic "get" function that allows you to get the current type on hand. Users have to explicitly cast the type they want.
```C++
Variant v1{ 5 };
if (v1.type() == typeid(int))
	v1.get_value<int>();		// returns the reference to int 5
// It is, however, to perform the following:
v1.get_value<float>();                  // does not throws an error but might have segmentation fault if used as an assignment
```
### API
- Constructors
```C++
Variant();                               // Default Constructor
Variant(const Variant&);                 // Copy Constructor
Variant(Variant&&);                      // Move Constructor
template <typename T, typename U = std::decay_t<T>>
Variant(const T&);                       // Conversion Constructor
```
- Member Functions
```C++
Variant& operator=(const Variant&);      // Copy Assignment operator
Variant& operator=(Variant&&);           // Move Assignment operator
template <typename T>
Variant& operator=(const T& value);      // Conversion Assignment operator

/*
   Function name:                  get_pointer
   Overloaded functions available: const T* get_pointer() const
   Description:                    Attempts to convert the stored data into the type given as a pointer.
                                   If the type is not convertible, it will return a nullptr
*/
template <typename T>
T* get_pointer();

/*
   Function name:                  get_value
   Overloaded functions available: const T& get_value() const
   Description:                    Attempts to convert the stored data into the type given.
                                   If the type is not convertible, it will return a rubbish value
*/
template <typename T, typename U = std::decay_t<T>>
U& get_value();

/*
   Function name:                  set_value
   Overloaded functions available: -
   Input:                          any type of value that is supported by Variant
   Description:                    Converts the current stored data into the specified value given in the parameter.
                                   Do not that this function does not check if the passed in type is supported.
*/
template <typename T>
void set_value(const T& value);

bool has_value() const;                    // Checks if Variant holds a value
unsigned index() const;                    // Returns the index at which Variant currently holds as a type
operator bool() const;                     // Returns a boolean to determine if Variant holds any value
template <typename T>
operator T() const;                        // Converts Variant into the type specified. However, this does not work for UDT
std::type_index type() const;              // Returns a std::type_index that the Variant currently holds

/*
   Function name:                  operator==
   Overloaded functions available: bool operator==(const Variant&)
   Input:                          any type of value
   Description:                    Compares the type (and then value) of Variant and the passed in parameter
*/
template <typename T>
bool operator==(const T& value);

/*
   Function name:                  operator!=
   Overloaded functions available: bool operator!=(const Variant&)
   Input:                          any type of value
   Description:                    Compares the type (and then value) of Variant and the passed in parameter
*/
template <typename T>
bool operator!=(const T& value);

bool operator==(const Variant& rhs);
bool operator!=(const Variant& rhs);

template <typename T
bool same_type(const T& value) const;       // Checks if the given value and Variant have the same type
```
- Non-Member Functions
```C++
/*
   Function name:                  LegalVariant(T&&)
   Overloaded functions available: -
   Input:                          any type of value
   Description:                    Determines if the value being passed in is supported by Variant
*/
template <typename T, typename U = std::decay_t<T>>
bool LegalVariant(T&&);
/*
   Function name:                  visit
   Overloaded functions available: -
   Input:                          std::tuple, size_t, typename Functor
   Description:                    Used in conjunction with tuple (similar to how std::visit works). To be used when the return type is common type
*/
template <typename Functor, typename ... Args>
decltype(auto) visit(std::tuple<Args...>& tup, size_t index, Functor func);
```


# Limitations
- Not able to support types with pointers (except for char* which is automatically converted to string)
- Not able to support function pointers
