#ifndef NAUTYPP_IMPL_HPP
#define NAUTYPP_IMPL_HPP

namespace nautypp {
bool EdgeIterator::operator==(const EdgeIterator& other) const {
    return std::addressof(graph) == std::addressof(other.graph)
       and v == other.v
       and gv == other.gv
       and w == other.w;
}

bool EdgeIterator::operator!=(const EdgeIterator& other) const {
    return not (*this == other);
}

EdgeIterator& EdgeIterator::operator=(EdgeIterator&& other) {
    if(std::addressof(graph) != std::addressof(other.graph))
        throw std::runtime_error(
            "Cannot reassign EdgeIterator to a new graph"
        );
    v = other.v;
    w = other.w;
    gv = other.gv;
    return *this;
}

EdgeIterator& EdgeIterator::operator++() {
    if(gv > 0) {
        TAKEBIT(w, gv);
    } else {
        w = WORDSIZE;
    }
    return *this;
}

std::pair<Vertex, Vertex> EdgeIterator::operator*() const {
    return {v, w};
}

bool AllEdgeIterator::operator==(const AllEdgeIterator& other) const {
    return it == other.it;
}

bool AllEdgeIterator::operator!=(const AllEdgeIterator& other) const {
    return not (*this == other);
}

AllEdgeIterator& AllEdgeIterator::operator++() {
    next();
    return *this;
}

void AllEdgeIterator::next() {
    ++it;
    auto [v, w] = *it;
    while(w == WORDSIZE and v+1 < graph.V()) {
        ++v;
        it = EdgeIterator(
            graph, v,
            *GRAPHROW(graph.g, v, graph.__get_m()) & MASKS[v]
        );
        std::tie(v, w) = *it;
    }
}

std::pair<Vertex, Vertex> AllEdgeIterator::operator*() const {
    return *it;
}

void DegreeProperty::compute() {
    value = std::popcount(*GRAPHROW(
        static_cast<const nauty_graph_t*>(*graph),
        v,
        graph->__get_m()
    ));
}

void EdgeProperty::compute() {
    value = 0;
    for(size_t v{0}; v < graph->V(); ++v)
        value += graph->degree(v);
    value >>= 1;  // sum(d(v)) == 2E
}

void _CliquerGraphProperty::compute() {
    clear();
    value = graph_new(graph->V());
    for(auto [v, w] : graph->edges()) {
        GRAPH_ADD_EDGE(value, v, w);
    }
}

ConnectedComponents::ConnectedComponents(const Graph& graph):
        G{graph}, ids(graph.V(), UNVISITED), nb_components{0} {
    _run();
}

void ConnectedComponents::_run() {
    for(Vertex v{0}; v < G.V(); ++v) {
        if(ids[v] == UNVISITED) {
            _run(v);
            ++nb_components;
        }
    }
}

#ifdef OSTREAM_LL_GRAPH
static std::ostream& operator<<(std::ostream& os, const nautypp::Graph& G) {
    os << "Graph(";
    bool first{true};
    for(auto [v, w] : G.edges()) {
        if(first)
            first = false;
        else
            os << ", ";
        os << v << "-" << w;
    }
    return os << ")";
}
#endif
#ifdef OSTREAM_LL_CLIQUE
static std::ostream& operator<<(std::ostream& os, const nautypp::Cliquer::Set& S) {
    os << '{';
    bool first{true};
    for(auto v : S) {
        if(first)
            first = false;
        else
            os << ", ";
        os << v;
    }
    return os << '}';
}
#endif
}

#endif
