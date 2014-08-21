export
# Compile commands
ifeq ($(shell clang > /dev/null 2> /dev/null; echo $$?), $(shell echo 1))
CC = clang
else
CC = gcc
endif
ifeq ($(shell clang++ > /dev/null 2> /dev/null; echo $$?), $(shell echo 1))
CXX = clang++
LINK = clang++
else
CXX = g++
LINK = g++
endif

CFLAGS = -Wall -Wextra -Wabi -Weffc++ -Wshadow -Wfloat-equal -Wno-unused-result -O3 -c
CXXFLAGS := $(CFLAGS) -std=c++11
LFLAGS = -Wall -Wextra -Wabi -Weffc++ -Wshadow -Wfloat-equal -Wno-unused-result -s -O3 -std=c++11 -pthread

# Shell commands
MV = mv -f
CP = cp -f -r
UPDATE = $(CP) -u
RM = rm -f
MKDIR = mkdir -p

.PHONY: all debug clean

all: ../sd

debug:
	make all CC='$(CC) -DDEBUG' CXX='$(CXX) -DDEBUG' LINK='$(LINK) -DDEBUG'

full-debug:
	make all CC='$(CC) -DDEBUG' CXX='$(CXX) -DDEBUG' LINK='$(LINK) -DDEBUG' CFLAGS='-Wall -Wextra -Wabi -Weffc++ -Wshadow -Wfloat-equal -Wno-unused-result -O0 -g -c' LFLAGS=' -Wall -Wextra -Wabi -Weffc++ -Wshadow -Wfloat-equal -Wno-unused-result -O0 -g -std=c++11 -pthread'

../sd: main.o functions.o aho.o
	$(LINK) $^ $(LFLAGS) -o $@

main.o: main.cpp functions.hpp Algorithms/text/aho.hpp Algorithms/text/trie.hpp
	$(CXX) $< $(CXXFLAGS) -o $@

functions.o: functions.cpp functions.hpp
	$(CXX) $< $(CXXFLAGS) -o $@

aho.o: Algorithms/text/aho.cpp Algorithms/text/aho.hpp
	$(CXX) $< $(CXXFLAGS) -o $@

clean:
	$(RM) *.o

mrproper: clean
	$(RM) ../CTH