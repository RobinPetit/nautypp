#ifndef NAUTYPP_ITERATORS_HPP
#define NAUTYPP_ITERATORS_HPP

#include <nautypp/aliases.hpp>

#include <array>
#include <utility>
#include <vector>

namespace nautypp {

class Graph;

/*      *************** Iterables and Iterators ***************      */

/// \class EdgeIterator
/// \brief Iterator over edges incident to some vertex
///
/// **Example**:
/// \include iterators/neighbours.cpp
class EdgeIterator {
public:
    EdgeIterator() = delete;
    EdgeIterator(const Graph&, Vertex vertex, bool end=false);
    EdgeIterator(const Graph&, Vertex vertex, setword Nv);
    inline bool operator==(const EdgeIterator&) const;
    inline bool operator!=(const EdgeIterator&) const;
    inline EdgeIterator& operator=(EdgeIterator&& other);
    inline EdgeIterator& operator++();
    inline std::pair<Vertex, Vertex> operator*() const;
private:
    const Graph& graph;
    Vertex v, w;
    setword gv;
};

/// \class AllEdgeIterator
/// \brief Iterator over all edges of a graph
///
/// See Graph::edges()
/// **Example**:
/// \include iterators/neighbours.cpp
class AllEdgeIterator {
public:
    AllEdgeIterator() = delete;
    AllEdgeIterator(const Graph&, bool end=false);
    inline bool operator==(const AllEdgeIterator&) const;
    inline bool operator!=(const AllEdgeIterator&) const;
    inline AllEdgeIterator& operator++();
    inline std::pair<Vertex, Vertex> operator*() const;
private:
    const Graph& graph;
    EdgeIterator it;

    inline void next();

    /// MASKS[i] = (0xFF...FF) >> (i+1)
    static constexpr std::array<setword, WORDSIZE> MASKS{[]{
        std::array<setword, WORDSIZE> ret{};
        auto value{static_cast<setword>(-1)};
        for(size_t i{0}; i < WORDSIZE; ++i)
            ret[i] = (value >>= 1);
        //setword value{-1};
        return ret;
    }()};
};

/// \class Edges
/// \brief Iterable over all edges incident to some vertex
///
/// See EdgeIterator and Graph::edges_incident_to()
class Edges {
public:
    Edges() = delete;
    Edges(const Graph& G, Vertex vertex):
            graph{G}, v{vertex} {
    }

    inline EdgeIterator begin() const {
        return {graph, v};
    }
    inline EdgeIterator end() const {
        return {graph, v, true};
    }
private:
    const Graph& graph;
    Vertex v;
};

/// \class AllEdges
/// \brief Iterable over all edges of a graph
///
/// See AllEdgeIterator and Graph::edges()
class AllEdges {
public:
    AllEdges() = delete;
    AllEdges(const Graph& G):
            graph{G} {
    }

    inline AllEdgeIterator begin() const {
        return {graph, false};
    }
    inline AllEdgeIterator end() const {
        return {graph, true};
    }

    operator std::vector<std::pair<Vertex, Vertex>>() const;

    inline std::vector<std::pair<Vertex, Vertex>> as_vector() const {
        return *this;
    }
private:
    const Graph& graph;
};

class NeighbourIterator {
public:
    NeighbourIterator() = delete;
    NeighbourIterator(const Graph& G, Vertex vertex, bool end=false):
            it(G, vertex, end) {
    }
    NeighbourIterator(const Graph& G, Vertex vertex, setword Nv):
            it(G, vertex, Nv) {
    }
    inline bool operator==(const NeighbourIterator& other) const {
        return it == other.it;
    }
    inline bool operator!=(const NeighbourIterator& other) const {
        return it != other.it;
    }
    inline NeighbourIterator& operator=(NeighbourIterator&& other) {
        it = std::move(other.it);
        return *this;
    }
    inline NeighbourIterator& operator++() {
        ++it;
        return *this;
    }
    inline Vertex operator*() const {
        return (*it).second;
    }
private:
    EdgeIterator it;
};

class Neighbours {
public:
    Neighbours() = delete;
    Neighbours(const Graph& G, Vertex vertex):
            graph{G}, v{vertex} {
    }

    inline NeighbourIterator begin() const {
        return {graph, v};
    }
    inline NeighbourIterator end() const  {
        return {graph, v, true};
    }

    operator std::vector<Vertex>() const;
private:
    const Graph& graph;
    Vertex v;
};

}

#endif
