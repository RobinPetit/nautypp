CC=g++
FLAGS=-std=c++20 -g -O0 -Wall -Wextra -I ~/includes/ -L ~/lib/ -lnautypp -pthread -fsanitize=address

EXAMPLES=bin/multithreaded_count_triangle_free_graphs bin/multithreaded_heavy_callback \
		 bin/multithreaded_cliquer bin/multithreaded_graph_reader \
         bin/iterators_neighbours bin/degree_degrees \
		 bin/cliquer bin/planar

all: ${EXAMPLES}

run: ${EXAMPLES}
	for test in ${EXAMPLES}; do echo Running test $$test:; $$test; done

bin/%: obj/%.o
	${CC} -o $@ $^ ${FLAGS}

obj/multithreaded_%.o: multithreaded/%.cpp
	${CC} -o $@ $< -c ${FLAGS}

obj/iterators_%.o: iterators/%.cpp
	${CC} -o $@ $< -c ${FLAGS}

obj/degree_%.o: degree/%.cpp
	${CC} -o $@ $< -c ${FLAGS}

obj/%.o: %.cpp
	${CC} -o $@ $< -c ${FLAGS}

clear:
	rm -f ${EXAMPLES}
	rm -f obj/*.o

.PRECIOUS: obj/%.o
