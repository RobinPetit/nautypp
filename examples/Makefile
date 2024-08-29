CC=g++
FLAGS=-std=c++20 -g -O0 -Wall -Wextra -I ~/includes/ -L ~/lib/ -lnautypp -pthread

EXAMPLES=bin/count_triangle_free_graphs

all: ${EXAMPLES}

bin/%: count_triangle_free_graphs.o
	${CC} -o $@ $^ ${FLAGS}

%.o: %.cpp
	${CC} -o $@ $< -c ${FLAGS}
