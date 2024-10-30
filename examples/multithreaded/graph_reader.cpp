#include <algorithm>
#include <cstdlib>
#include <iterator>
#include <nautypp/nautypp>
#include <sstream>

using namespace nautypp;

static constexpr char const *  file_path{"tmp.txt"};
static constexpr char const *  cmd{"geng -t"};
static constexpr size_t        nb_vertices{5};

inline void init_file() {
    std::stringstream ss;
    ss << cmd << " " << nb_vertices << " -q " << file_path;
    std::system(ss.str().c_str());
}

inline void remove_file() {
    std::remove(file_path);
}

std::vector<Vertex> common_neighbours(const Graph& G, Vertex v, Vertex w) {
    std::vector<Vertex> ret;
    std::vector<Vertex> Nv{G.neighbours_of(v)};
    std::vector<Vertex> Nw{G.neighbours_of(w)};
    std::set_intersection(
        Nv.begin(), Nv.end(),
        Nw.begin(), Nw.end(),
        std::back_inserter(ret)
    );
    return ret;
}

bool contains_triangle(const Graph& G) {
    for(Vertex v{0}; v < G.V(); ++v)
        for(Vertex w : G.neighbours_of(v))
            if(common_neighbours(G, v, w).size() > 0)
                return true;
    return false;
}

int main() {
    init_file();
    Nauty nauty;
    std::atomic_bool found_triangle{false};
    nauty.run_async(
        [&found_triangle](const Graph& G) {
            if(contains_triangle(G)) {
                std::cerr << "Found a triangle\n";
                found_triangle = true;
            }
        },
        file_path,
        nb_vertices
    );
    remove_file();
    if(not found_triangle)
        std::cout << "All good!\n";
    return EXIT_SUCCESS;
}
