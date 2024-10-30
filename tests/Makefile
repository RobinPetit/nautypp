CXXFLAGS=-std=c++20 -MMD -Wall -Wextra -O3
#CXXFLAGS=-std=c++20 -MMD -Wall -Wextra -g -fsanitize=address

SRC=$(wildcard *.cpp)
OBJ=$(patsubst %.cpp,obj/%.o,${SRC})
EXE=bin/test

run: build geng_4_biconnected.graph6 geng_4_biconnected.sparse6
	${EXE}

build: ${EXE}

${EXE}: ${OBJ}
	mkdir -p bin/
	${CXX} -o $@ $^ ${CXXFLAGS} -lnautypp

obj/%.o: %.cpp
	mkdir -p obj/
	g++ -c -o $@ $< ${CXXFLAGS}

-include obj/*.d

geng_4_biconnected.graph6:
	geng -C 4 -g $@

geng_4_biconnected.sparse6:
	geng -C 4 -s $@

clear:
	rm -f obj/*
	rm -f ${EXE}

.PHONY: clear
