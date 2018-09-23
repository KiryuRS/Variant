
CC = g++
CFLAGS = -Wall -Wextra -Werror -std=c++17
ALL = var

all: $(ALL)

var : test_case_1.cpp Makefile
	$(CC) $(CFLAGS) -o var.exe test_case_1.cpp

clean:
	$(RM) $(ALL) *.o

test: all
	bash test
