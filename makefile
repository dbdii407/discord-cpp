LIBFLAGS := $(shell pkg-config --cflags --libs libcurl openssl)
ARGS = -Wall -g -std=gnu++20 -lpthread -Wfatal-errors -Wno-sign-compare -Wno-reorder -Wno-sequence-point

MAIN = index.cpp
SRC = src
OUT = dist
CC = g++

HEADERS = -Iincludes/*.hpp -Iincludes/pqt/*.hpp
SRCS = ./src/pqt/*.cpp

main:
	$(CC) $(ARGS) $(HEADERS) $(LIBFLAGS) index.cpp -o ./dist/app

test:
	$(CC) $(ARGS) $(HEADERS) $(LIBFLAGS) test.cpp -o ./dist/test