CC=g++
FLAGS=-std=c++20 -g -O0 -Wall -Wextra -I ~/includes/ -L ~/lib/ -lnautypp -pthread

EXAMPLES=bin/multithreaded_count_triangle_free_graphs bin/multithreaded_heavy_callback \
         bin/iterators_neighbours bin/degree_degrees

all: ${EXAMPLES}

bin/%: obj/%.o
	${CC} -o $@ $^ ${FLAGS}

obj/multithreaded_%.o: multithreaded/%.cpp
	${CC} -o $@ $< -c ${FLAGS}

obj/iterators_%.o: iterators/%.cpp
	${CC} -o $@ $< -c ${FLAGS}

obj/degree_%.o: degree/%.cpp
	${CC} -o $@ $< -c ${FLAGS}
