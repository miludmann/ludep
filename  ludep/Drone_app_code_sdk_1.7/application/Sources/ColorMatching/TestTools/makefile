CXX=g++ 
CXXFLAGS=-Wall

CPPFLAGS=-g
LDFLAGS=-g
LDLIBS=-pthread

all:Client Server

Client: ../client.o client_main.o
	g++ $(LDFLAGS) -o Client ../client.o client_main.o $(LDLIBS) 

Server: server.o
	g++ $(LDFLAGS) -o Server server.o $(LDLIBS) 

client.o: ../client.cpp ../client.hpp
	g++ $(CPPFLAGS) -c ../client.cpp
	
client_main.o: client_main.c
	g++ $(CPPFLAGS) -c client_main.c 
	
server.o:
	g++ $(CPPFLAGS) -c server.c 

clean:
	rm -f Client ../client.o client_main.o 
	rm -f Server server.o
