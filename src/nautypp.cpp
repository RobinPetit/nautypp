#include <string>
#include <iostream>
#include <bitset>

#include "nauty.hpp"

using namespace nautypp;

void _gentreeg_callback(FILE* f, int* par, int n) {
    Nauty::get_container()->_add_gentree_tree(par, n);
    (void)f;
}

void _geng_callback(FILE* f, graph* g, int n) {
    Nauty::get_container()->emplace(g, n, true);
    (void)f;
}

namespace nautypp {
void NautyThreadLauncher::start_gentreeg(int argc, char** argv) {
    _gentreeg_main(argc, argv);
    Nauty::get_container()->set_over();
}

std::unique_ptr<NautyContainer> Nauty::_container;

/***** EdgeIterator *****/

EdgeIterator::EdgeIterator(const Graph& G, Vertex vertex, bool end):
        graph{G},
        v{vertex}, w{0},
        gv{
            end
            ? 0
            : *GRAPHROW(G.g, v, G.__get_m())
        } {
    ++*this;
}

AllEdgeIterator::AllEdgeIterator(const Graph& G, bool end):
        graph{G}, it(G, end ? G.V()-1 : 0, end) {
}

/***** Edges *****/

Edges::Edges(const Graph& G, xword vertex):
        graph{G}, v{vertex} {
}

AllEdges::AllEdges(const Graph& G):
        graph{G} {
}

AllEdges::operator std::vector<std::pair<Vertex, Vertex>>() const {
    std::vector<std::pair<Vertex, Vertex>> ret(graph.E(), {0, 0});
    for(size_t i{0}; auto [v, w] : *this)
        ret[i++] = {v, w};
    return ret;
}

/***** Graph *****/

Graph::Graph(const graph* G, size_t V):
        Graph(const_cast<graph*>(G), V, true) {
}

Graph::Graph(graph* G, size_t V, bool copy):
        n{V}, host{copy},
        g{copy ? nullptr : G},
        nb_edges(*this), degrees() {
    if(copy)
        assign_from(G, V);
    init_degrees();
}

#ifdef NAUTYPP_SGO
Graph::Graph(size_t V):
        n{V}, host{n > NAUTYPP_SMALL_GRAPH_SIZE},
        g{host ? new graph[_m*V] : __small_graph_buffer},
        nb_edges(*this), degrees() {
    for(Vertex v{0}; v < n; ++v)
        g[v] = 0;
    init_degrees();
}

Graph::Graph(Graph&& G):
        n{G.n}, host{G.host},
        g{
            G.g != &G.__small_graph_buffer[0]
            ? G.g
            : __small_graph_buffer
        },
        nb_edges{std::move(G.nb_edges)}, degrees(std::move(G.degrees)) {
    G.host = false;
    if(g == __small_graph_buffer)
        memcpy(g, G.g, _m*G.n*sizeof(graph));
}
#else
Graph::Graph(size_t V):
        n{V}, host{true},
        g{new graph[_m * V]},
        nb_edges(*this), degrees() {
    for(Vertex v{0}; v < _m*V; ++v)
        g[v] = 0;
    init_degrees();
}

Graph::Graph(Graph&& G):
        n{G.n}, host{G.host},
        g{G.g}, nb_edges(std::move(G.nb_edges)),
        degrees(std::move(G.degrees)) {
    G.host = false;
    G.g = nullptr;
    for(size_t v{0}; v < n; ++v)
        degrees[v].reset_graph(this);
    nb_edges.reset_graph(this);
}
#endif

Graph::Graph(int* parents, size_t V):
        n{V}, host{true}, g{nullptr},
        nb_edges(*this), degrees() {
    g = new graph[_m*n]{0};
    for(size_t v{2}; v <= V; ++v) {
        ADDONEEDGE(
            g, v-1,
            parents[v]-1,  // /!\ gentreeg uses 1..n for node indices
            _m
        );
    }
}

Graph& Graph::operator=(Graph&& other) {
    reset();
    g = other.g;
    n = other.n;
    host = other.host;
    nb_edges = std::move(other.nb_edges);
    degrees = std::move(other.degrees);
    other.host = false;
    return *this;
}

/***** properties *****/

/*
NautyIterator NautyIterable::begin() const {
    return NautyIterator(*Nauty::get_container().get(), buffer_size, false);
}

NautyIterator NautyIterable::end() const {
    return NautyIterator(*Nauty::get_container().get(), buffer_size, true);
}
*/
}
