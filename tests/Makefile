#CXXFLAGS=-std=c++20 -MMD -Wall -Wextra -O3
CXXFLAGS=-std=c++20 -MMD -Wall -Wextra -g

SRC=$(wildcard *.cpp)
OBJ=$(patsubst %.cpp,obj/%.o,${SRC})
EXE=bin/test

run: build
	${EXE}

build: ${EXE}

${EXE}: ${OBJ}
	mkdir -p bin/
	${CXX} -o $@ $^ ${CXXFLAGS} -lnautypp

obj/%.o: %.cpp
	mkdir -p obj/
	g++ -c -o $@ $< ${CXXFLAGS}

-include obj/*.d

clear:
	rm -f obj/*
	rm -f ${EXE}

.PHONY: clear
