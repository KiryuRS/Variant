
CC = g++
C17FLAGS = -Wall -Wextra -Werror -std=c++17
C14FLAGS = -Wall -Wextra -Werror -std=c++14
ALL = var fun_var var_14

all: $(ALL)

var : src/test_case_1.cpp Makefile
	$(CC) $(C17FLAGS) -o var.exe src/test_case_1.cpp

fun_var : src/fun_test_case.cpp Makefile
	$(CC) $(C17FLAGS) -o fun_var.exe src/fun_test_case.cpp

var_14 : src/test_case_1.cpp Makefile
	$(CC) $(C14FLAGS) -o var_14.exe src/test_case_1.cpp

clean:
	$(RM) $(ALL) *.o

test: all
	./var.exe
	./fun_var.exe
	./var_14.exe
