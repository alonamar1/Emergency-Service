CFLAGS := -c -Wall -Weffc++ -g -std=c++11 -Iinclude
LDFLAGS := -lboost_system -lpthread

all: bin/StompClient.exe run

bin/StompClient.exe: bin/ConnectionHandler.o bin/StompClient.o bin/event.o bin/dataBaseClient.o
	g++ -o bin/StompClient bin/ConnectionHandler.o bin/StompClient.o bin/event.o bin/dataBaseClient.o $(LDFLAGS)

bin/ConnectionHandler.o: src/ConnectionHandler.cpp
	g++ $(CFLAGS) -o bin/ConnectionHandler.o src/ConnectionHandler.cpp

bin/StompClient.o: src/StompClient.cpp
	g++ $(CFLAGS) -o bin/StompClient.o src/StompClient.cpp

bin/event.o: src/event.cpp
	g++ $(CFLAGS) -o bin/event.o src/event.cpp

bin/dataBaseClient.o: src/dataBaseClient.cpp
	g++ $(CFLAGS) -o bin/dataBaseClient.o src/dataBaseClient.cpp

run: bin/StompClient
	./bin/StompESClient

.PHONY: clean

clean:
	rm -f bin/*
