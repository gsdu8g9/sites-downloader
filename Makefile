CXX = g++
ARCH = 64
CXXFLAGS = -s -O3 -Wall -m$(ARCH)
LDFLAGS = $(CXXFLAGS)
RM = rm -f

all: sites-downloader

sites-downloader: main.cpp
	$(CXX) $< $(CXXFLAGS) -o $@