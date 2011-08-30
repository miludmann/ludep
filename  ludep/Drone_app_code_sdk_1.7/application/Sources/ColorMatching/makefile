CXX=g++ 
CXXFLAGS=-Wall

CPPFLAGS=-g 
LDFLAGS=-g
LDLIBS=-lhighgui -pthread -I/usr/include/opencv/ 

all:ColorMatcher

ColorMatcher: client.o colorMatcher.o Main.o
	g++ $(LDFLAGS) -o ColorMatcher client.o colorMatcher.o Main.o $(LDLIBS) 

client.o: client.cpp client.hpp
	g++ $(CPPFLAGS) -c client.cpp

colorMatcher.o: colorMatcher.cpp colorMatcher.hpp
	g++ $(CPPFLAGS) -c colorMatcher.cpp 
	
Main.o: Main.cpp
	g++ $(CPPFLAGS) -c Main.cpp 

clean:
	rm -f ColorMatcher client.o colorMatcher.o Main.o
