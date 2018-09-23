
#include <array>			// std::array
#include <vector>			// std::vector

#define VARIANTTYPES \
char, short, int, long, long long, size_t, \
unsigned char, unsigned short, unsigned int, unsigned long, unsigned long long, \
short, double, \
std::string, std::vector<int>, std::array<int, 5>, \
std::vector<std::array<float, 5>>

#include "Variant.h"

int main(void)
{
	Variant crazy;
	using messed_up = std::vector<std::array<float, 5>>;
	messed_up stuff;
	crazy = stuff;

	std::vector<int> not_so_bad;
	std::array<int, 5> not_too_bad;

	crazy = not_so_bad;
	crazy = not_too_bad;

	std::cout << "No throws!" << std::endl;
}
