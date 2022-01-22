
CC = g++
C17FLAGS = -Wall -Wextra -Werror -std=c++17
C14FLAGS = -Wall -Wextra -Werror -std=c++14

all: tests/test_variant.cpp Makefile
	$(CC) $(C17FLAGS) -o test.exe tests/test_variant.cpp

clean:
	$(RM) $(ALL) *.o

test: all
	./test.exe
