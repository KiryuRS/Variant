
CC = g++
CFLAGS = -Wall -Wextra -Werror -std=c++17
ALL = var fun_var

all: $(ALL)

var : src/test_case_1.cpp Makefile
	$(CC) $(CFLAGS) -o var.exe src/test_case_1.cpp

fun_var : src/fun_test_case.cpp Makefile
	$(CC) $(CFLAGS) -o fun_var.exe src/fun_test_case.cpp

clean:
	$(RM) $(ALL) *.o

test: all
	./var.exe
	./fun_var.exe