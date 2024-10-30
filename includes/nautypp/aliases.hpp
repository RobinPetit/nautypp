#ifndef NAUTYPP_ALIASES_HPP
#define NAUTYPP_ALIASES_HPP

#include <limits>

#include <nauty/gtools.h>

typedef std::size_t size_t;

namespace nautypp {

// From geng.c
typedef setword xword;
typedef graph nauty_graph_t;
typedef xword Vertex;
static constexpr auto NO_VERTEX{std::numeric_limits<Vertex>::max()};

class Graph;

/* ******************** Concepts ******************** */
template <typename T>
concept GraphRefFunctionType = requires(T obj, Graph& G) {
    { obj(G) };
};
template <typename T>
concept GraphConstRefFunctionType = requires(T obj, const Graph& G) {
    { obj(G) };
};
template <typename T>
concept GraphFunctionType = GraphRefFunctionType<T>
                         or GraphConstRefFunctionType<T>;

}
#endif
