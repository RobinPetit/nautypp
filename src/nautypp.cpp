#include "nautypp/aliases.hpp"
#include "nautypp/iterators.hpp"
#include "nautypp/nauty.hpp"

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

std::unique_ptr<NautyContainer> Nauty::_container;

/***** EdgeIterator *****/

EdgeIterator::EdgeIterator(const Graph& G, Vertex vertex, bool end):
        graph{G},
        v{vertex}, w{end ? NO_VERTEX : 0},
        gv{GRAPHROW(G.g, v, G.__get_m())} {
    if(w != NO_VERTEX and not ISELEMENT(gv, w))
    ++*this;
}

EdgeIterator& EdgeIterator::operator++() {
    if(w != NO_VERTEX) {
        int x = nextelement(gv, graph.__get_m(), w);
        w = (x < 0) ? NO_VERTEX : x;
    }
    return *this;
}

AllEdgeIterator::AllEdgeIterator(const Graph& G, bool end):
        graph{G}, v{end ? G.V()-1 : 0},
        it(G, v, end), it_end(G, v, true) {
    if(it == it_end)
        next();
    else
        it.goto_afeter_v();
}

void AllEdgeIterator::next() {
    ++it;
    while(it == it_end and v < graph.V()-1) {
        ++v;
        it = EdgeIterator(graph, v, false);
        it.goto_afeter_v();
        it_end = EdgeIterator(graph, v, true);
    }
}

/***** Edges *****/

AllEdges::operator std::vector<std::pair<Vertex, Vertex>>() const {
    std::vector<std::pair<Vertex, Vertex>> ret(graph.E(), {0, 0});
    for(size_t i{0}; auto [v, w] : *this)
        ret[i++] = {v, w};
    return ret;
}
Neighbours::operator std::vector<Vertex>() const {
    std::vector<Vertex> ret;
    ret.reserve(graph.degree(v));
    for(auto w : *this)
        ret.push_back(w);
    return ret;
}

/***** Graph *****/

Graph::Graph(const graph* G, size_t V):
        Graph(const_cast<graph*>(G), V, true) {
}

Graph::Graph(graph* G, size_t V, bool copy):
        n{V}, host{copy},
        _m{SETWORDSNEEDED(n)},
        g{copy ? nullptr : G},
        nb_edges(*this), degrees(),
        _as_cliquer(*this) {
    if(copy)
        assign_from(G, V);
    init_degrees();
}

#ifdef NAUTYPP_SGO
Graph::Graph(size_t V):
        n{V}, host{n > NAUTYPP_SMALL_GRAPH_SIZE},
        _m{SETWORDSNEEDED(n)},
        g{host ? new graph[_m*V] : __small_graph_buffer},
        nb_edges(*this), degrees(),
        _as_cliquer(*this) {
    for(Vertex v{0}; v < n; ++v)
        g[v] = 0;
    init_degrees();
}

Graph::Graph(Graph&& G):
        n{G.n}, host{G.host},
        _m{SETWORDSNEEDED(n)},
        g{
            G.g != &G.__small_graph_buffer[0]
            ? G.g
            : __small_graph_buffer
        },
        nb_edges{std::move(G.nb_edges)},
        degrees(std::move(G.degrees)),
        _as_cliquer(*this) {
    G.host = false;
    if(g == __small_graph_buffer)
        memcpy(g, G.g, _m*G.n*sizeof(graph));
}
#else
Graph::Graph(size_t V):
        n{V}, host{true},
        _m{SETWORDSNEEDED(n)},
        g{new graph[_m * V]},
        nb_edges(*this), degrees(),
        _as_cliquer(*this) {
    for(Vertex v{0}; v < _m*V; ++v)
        g[v] = 0;
    init_degrees();
}

Graph::Graph(Graph&& G):
        n{G.n}, host{G.host},
        _m{G._m},
        g{G.g}, nb_edges(std::move(G.nb_edges)),
        degrees(std::move(G.degrees)),
        _as_cliquer(*this) {
    G.host = false;
    G.g = nullptr;
    for(size_t v{0}; v < n; ++v)
        degrees[v].reset_graph(this);
    nb_edges.reset_graph(this);
}
#endif

Graph::Graph(int* parents, size_t V):
        n{V}, host{true},
        _m{SETWORDSNEEDED(n)}, g{nullptr},
        nb_edges(*this), degrees(),
        _as_cliquer(*this) {
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
    _m = other._m;
    g = other.g;
    n = other.n;
    host = other.host;
    nb_edges = std::move(other.nb_edges);
    degrees = std::move(other.degrees);
    other.host = false;
    _as_cliquer.set_stale();
    return *this;
}

Graph Graph::disjoint_union(const Graph& G1, const Graph& G2) {
    size_t new_size{G1.V() + G2.V()};
    Graph ret(new_size);
    for(auto [v, w] : G1.edges())
        ret.add_edge(v, w);
    for(auto [v, w] : G2.edges())
        ret.add_edge(v+G1.V(), w+G1.V());
    return ret;
}

inline void _nauty_complement(graph* g, int m, int n) {
    complement(g, m, n);
}

Graph Graph::complement() const {
    Graph ret(copy());
    _nauty_complement(static_cast<nauty_graph_t*>(ret), Graph::__get_m(), V());
    return ret;
}

Cliquer::Set Graph::find_some_clique(
        size_t minsize, size_t maxsize,
        bool maximal) const {
    cliquer_graph_t* graph{*this};
    set_t clique = clique_unweighted_find_single(
        graph, minsize, maxsize, maximal, NULL
    );
    Cliquer::Set ret{clique};
    set_free(clique);
    return ret;
}

template <typename T>
static inline std::remove_reference_t<std::remove_cv_t<T>>&
_get_user_data_as_ref(clique_options* opts) {
    return *static_cast<std::remove_reference_t<std::remove_cv_t<T>>*>(
        opts->user_data
    );
}

static inline boolean _add_clique(set_t clique, cliquer_graph_t*, clique_options* opts) {
    _get_user_data_as_ref<std::vector<Cliquer::Set>>(opts).push_back(
        clique
    );
    return true;
}

std::vector<Cliquer::Set> Graph::get_all_cliques(size_t minsize, size_t maxsize, bool maximal) const {
    std::vector<Cliquer::Set> cliques;
    clique_options opts = {
        .reorder_function=NULL,
        .reorder_map=NULL,
        .time_function=NULL,
        .output=NULL,
        .user_function=_add_clique,
        .user_data=&cliques,
        .clique_list=NULL,
        .clique_list_length=0
    };
    clique_unweighted_find_all(
        static_cast<cliquer_graph_t*>(*this),
        minsize, maxsize, maximal, &opts
    );
    return cliques;
}

static inline boolean _set_callback(set_t clique, cliquer_graph_t*, clique_options* opts) {
    typedef std::function<bool(const Cliquer::Set&)> SetCallback;
    return _get_user_data_as_ref<SetCallback>(opts)(
        clique
    );
}

size_t Graph::apply_to_cliques(size_t minsize, size_t maxsize, bool maximal,
        std::function<bool(const Cliquer::Set&)> callback) const {
    clique_options opts = {
        .reorder_function=NULL,
        .reorder_map=NULL,
        .time_function=NULL,
        .output=NULL,
        .user_function=_set_callback,
        .user_data=&callback,
        .clique_list=NULL,
        .clique_list_length=0
    };
    return clique_unweighted_find_all(
        static_cast<cliquer_graph_t*>(*this),
        minsize, maxsize, maximal, &opts
    );
}

static inline boolean _vector_callback(set_t clique, cliquer_graph_t*, clique_options* opts) {
    typedef std::function<bool(const std::vector<Vertex>&)> VectorCallback;
    return _get_user_data_as_ref<VectorCallback>(opts)(
        static_cast<std::vector<Vertex>>(Cliquer::Set(clique, false))
    );
}

size_t Graph::apply_to_cliques(size_t minsize, size_t maxsize, bool maximal,
        std::function<bool(const std::vector<Vertex>&)> callback) const {
    clique_options opts = {
        .reorder_function=NULL,
        .reorder_map=NULL,
        .time_function=NULL,
        .output=NULL,
        .user_function=_vector_callback,
        .user_data=&callback,
        .clique_list=NULL,
        .clique_list_length=0
    };
    return clique_unweighted_find_all(
        static_cast<cliquer_graph_t*>(*this),
        minsize, maxsize, maximal, &opts
    );
}

template <typename T>
static inline T pop_from(std::vector<T>& v) {
    T&& ret{std::move(v.back())};
    v.pop_back();
    return ret;
}

void ConnectedComponents::_run(Vertex v) {
    std::vector<Vertex> stack;
    stack.push_back(v);
    while(not stack.empty()) {
        auto v{pop_from(stack)};
        ids[v] = nb_components;
        for(auto [_, w] : G.edges(v))
            if(ids[w] == UNVISITED)
                stack.push_back(w);
    }
}

}  // namespace nautypp
