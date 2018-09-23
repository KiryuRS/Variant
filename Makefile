
CC = g++
CFLAGS = -Wall -Wextra -Werror
ALL = var

all: $(ALL)

var : src/test_case_1.cpp Makefile
	$(CC) $(CFLAGS) -o var.exe src/test_case_1.cpp

clean:
	$(RM) $(ALL) *.o

test: all
	bash test
