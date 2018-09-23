
/*
	Required functions for UDT:
	- Default Constructor
	- operator==() overloading
*/

#include <tuple>			// std::tie
#include <iostream>			// std::iostream
struct Point
{
	float x = 0, y = 0;

	bool operator==(const Point& rhs) const
	{
		return std::tie(x, y) == std::tie(rhs.x, rhs.y);
	}

	friend std::ostream& operator<<(std::ostream& os, const Point& p)
	{
		os << "x: " << p.x << ", y: " << p.y; 
	}
};

#define VARIANTTYPES \
char, short, int, long, long long, size_t, \
unsigned char, unsigned short, unsigned int, unsigned long, unsigned long long, \
short, double, \
std::string, Point

// Include only after you have define all of the classes that you wish Variant to support
#include "../header/variant.h"

int main(void)
{
	Point p1, p2{ 5.f, 10.f };

	Variant v1;											// v1 does not contains any type
	v1 = p1;											// v1's type is now Point
	std::cout << v1.get_value<Point>() << "\n";			// Prints out - x: 0, y: 0
	std::cout << v1.type().name() << "\n";				// type() returns a std::type_index
	Variant v2{ v1 };									// v2 is now p1
	std::cout << std::boolalpha << "v1 == v2? " <<
			     (v1.type() == v2.type()) << "\n";		// Prints out - v1 == v2? true
	v2 = p2;											// v2 is now p2
	v1 = std::string{ "Hello World!" };					// v1 is now std::string
	std::cout << v1.get_value<std::string>() << "\n";	// Prints out - Hello World!
	std::cout << v2.get_value<Point>() << "\n";			// Prints out - x: 5.0, y: 10.0
}

