# Compile commands
ifeq ($(shell clang > /dev/null 2> /dev/null; echo $$?), $(shell echo 1))
export CC = clang
else
export CC = gcc
endif
ifeq ($(shell clang++ > /dev/null 2> /dev/null; echo $$?), $(shell echo 1))
export CXX = clang++
export LINK = clang++
else
export CXX = g++
export LINK = g++
endif

# Shell commands
export CFLAGS = -Wall -W -Wabi -Weffc++ -Wformat -Wshadow -Wsequence-point -Wuninitialized -Wfloat-equal -O3 -c
export CXXFLAGS = -Wall -W -Wabi -Weffc++ -Wformat -Wshadow -Wsequence-point -Wuninitialized -Wfloat-equal -O3 -c -std=c++11
export LFLAGS = -Wall -W -Wabi -Weffc++ -Wformat -Wshadow -Wsequence-point -Wuninitialized -Wfloat-equal -s -O3 -pthread -std=c++11
export RM = rm -f

.PHONY: all clean dist-clean

all: sites-downloader sd test

sites-downloader: main1.cpp
	$(LINK) $< $(LFLAGS) -o $@

sd: main.o functions.o aho.o
	$(LINK) $^ $(LFLAGS) -o $@

main.o: main.cpp functions.hpp aho.hpp trie.hpp
	$(CXX) $< $(CXXFLAGS) -o $@

functions.o: functions.cpp functions.hpp
	$(CXX) $< $(CXXFLAGS) -o $@

aho.o: aho.cpp aho.hpp
	$(CXX) $< $(CXXFLAGS) -o $@

test: test.cpp trie.hpp
	$(LINK) $< -Wall -W -Wabi -Weffc++ -Wformat -Wshadow -Wsequence-point -Wuninitialized -Wfloat-equal -s -O3 -o $@

clean:
	$(RM) *.o

dist-clean:
	$(RM) *.o
