CXXFLAGS = -g -Wall -Wfatal-errors -std=c++17

ALL = hello

all: $(ALL)

var: test_case_1.cpp Makefile
	$(CXX) $(CXXFLAGS) -o $@ $@.cpp

clean:
	$(RM) $(ALL) *.o

test: all
	bash test
