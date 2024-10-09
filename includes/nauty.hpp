#ifndef __NAUTY_WRAPPER_HPP__
#define __NAUTY_WRAPPER_HPP__

/// \file nauty.hpp
/// \author Robin Petit
/// \brief Wrapper for nauty's geng and gentreeg in C++

#include <algorithm>
#include <bit>
#include <cmath>
#include <functional>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#ifdef __linux__
#include <pthread.h>
#endif

/* C's _Thread_local is called thread_local in C++ */
#define _Thread_local thread_local
#include <nauty/gtools.h>

/************/
#define new __new_var__  // this C file has functions with local variables called new...
// this is pretty hacky but C++ cannot implicit-cast void*
#define calloc(n, s) (set_t)calloc((n), (s))
extern "C" {
#include <nauty/nautycliquer.h>
}
#undef calloc
#undef new
/************/

#ifdef NAUTYPP_SGO
# ifndef NAUTYPP_SMALL_GRAPH_SIZE
// graphs with less than that many vertices do not need to be dynamically
// allocated (similar to SSO)
#  define NAUTYPP_SMALL_GRAPH_SIZE 0
# endif
#endif

extern int _geng_main(int, char**);
extern int _gentreeg_main(int, char**);

static inline void rename_thread(std::thread& thread, const char* name) {
#ifdef __linux__
    pthread_setname_np(
        thread.native_handle(),
        name
    );
#else
    (void)(thread);
    (void)(name);
#endif
}

/// \namespace nautypp
/// \brief Namespace containing everything related to the wrapper
namespace nautypp {

typedef setword xword;  // from geng.c
typedef graph nauty_graph_t;
typedef xword Vertex;

typedef graph_t cliquer_graph_t;  // from nautycliquer.h

namespace Cliquer {
class Set {
public:
    class iterator {
    public:
        iterator() = delete;
        iterator(const iterator&) = delete;
        iterator(iterator&& other);
        iterator(const set_t s, Vertex v=0): set{s}, current{v} {
            _goto_next();
        }

        inline bool operator==(const iterator& other) const {
            return set == other.set and current == other.current;
        }
        inline bool operator!=(const iterator& other) const {
            return set != other.set or current != other.current;
        }

        inline Vertex operator*() const {
            return current;
        }
        inline iterator& operator++() {
            if(current < SET_MAX_SIZE(set))
                ++current;
            _goto_next();
            return *this;
        }
    private:
        const set_t set;
        Vertex current;

        inline void _goto_next() {
            while(current < SET_MAX_SIZE(set) and
                  not SET_CONTAINS(set, current))
                ++current;
        }
    };
    Set() = delete;
    Set(set_t set, bool copy=true): _vtx_set{nullptr}, host{false} {
        if(copy) {
            _vtx_set = set_duplicate(set);
            host = true;
        } else {
            _vtx_set = set;
        }
    }
    Set(Set&& other):
            _vtx_set{std::move(other._vtx_set)},
            host{other.host} {
        other.host = false;
    }

    ~Set() {
        if(host) {
            set_free(_vtx_set);
            _vtx_set = nullptr;
            host = false;
        }
    }

    inline iterator begin() const {
        return iterator(_vtx_set, 0);
    }
    inline iterator end() const {
        return iterator(_vtx_set, SET_MAX_SIZE(_vtx_set));
    }

    inline explicit operator std::vector<Vertex>() const {
        std::vector<Vertex> ret;
        ret.reserve(static_cast<size_t>(set_size(_vtx_set)));
        for(Vertex v : *this)
            ret.push_back(v);
        return ret;
    }
    inline size_t size() const {
        return static_cast<size_t>(set_size(_vtx_set));
    }
private:
    set_t _vtx_set;
    bool host;
};
}


class Graph;

/// \struct NautyParameters
/// \brief Wrapper for the parameters given to geng/gentreeg
struct NautyParameters {
    bool tree          = false;  ///< only generate trees (using gentreeg)
    bool connected     = true;   ///< only generate connected graphs (-c)
    bool biconnected   = false;  ///< only generate biconnected (2-connected) graphs (-C)
    bool triangle_free = false;  ///< only generate triangle-free graphs (-t)
    bool C4_free       = false;  ///< only generate \f$C_4\f$-free graphs (-f)
    bool C5_free       = false;  ///< only generate \f$C_5\f$-free graphs (-p)
    bool K4_free       = false;  ///< only generate \f$K_4\f$-free graphs (-k)
    bool chordal       = false;  ///< only generate chordal graphs (-T)
    bool split         = false;  ///< only generate split graphs (-S)
    bool perfect       = false;  ///< only generate perfect graphs (-P)
    bool claw_free     = false;  ///< only generate claw-free graphs (-F)
    bool bipartite     = false;  ///< only generate bipartite graphs (-b)

    int  V             = -1;  ///< minimum number of vertices
    int  Vmax          = -1;  ///< maximum number of vertices
    int  min_deg       = -1;  ///< minimum degree of vertices
    int  max_deg       = std::numeric_limits<int>::max();  ///< maximum degree of vertices
};


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

/* ******************** Types ******************** */

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
    Edges(const Graph& G, Vertex vertex);

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
    AllEdges(const Graph& G);

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

/*      *************** Graph Properties ***************      */

template <typename T>
class ComputableProperty {
public:
    ComputableProperty() = delete;
    ComputableProperty(const Graph& G):
            graph{std::addressof(G)}, computed{false}, value{} {
    }
    ComputableProperty(const ComputableProperty&) = delete;
    ComputableProperty(ComputableProperty&& other):
            graph{other.graph}, computed{other.computed},
            value{std::move(other.value)} {
    }

    ComputableProperty& operator=(const ComputableProperty&) = delete;
    ComputableProperty& operator=(ComputableProperty&& other) {
        graph = other.graph;
        computed = other.computed;
        value = std::move(other.value);
        return *this;
    }

    virtual ~ComputableProperty() = default;
    inline void set_stale() {
        computed = false;
    }

    inline T get() {
        if(not computed) {
            compute();
            computed = true;
        }
        return value;
    }

    inline operator bool() const {
        return computed;
    }
protected:
    friend class Graph;

    const Graph* graph;
    bool computed=false;
    T value;
    virtual void compute() = 0;

    inline void reset_graph(const Graph* new_graph) {
        graph = new_graph;
    }
};

class EdgeProperty final : public ComputableProperty<size_t> {
public:
    EdgeProperty(const Graph& G): ComputableProperty(G) {
    }
    EdgeProperty(EdgeProperty&& other):
            ComputableProperty<size_t>(std::move(other)) {
    }
    EdgeProperty& operator=(EdgeProperty&& other) {
        ComputableProperty<size_t>::operator=(std::move(other));
        return *this;
    }
    virtual ~EdgeProperty() = default;
protected:
    virtual inline void compute() override final;
};

class DegreeProperty final : public ComputableProperty<size_t> {
public:
    DegreeProperty(const Graph& G, Vertex vertex):
            ComputableProperty<size_t>(G), v{vertex} {
    }
    DegreeProperty(DegreeProperty&& other):
            ComputableProperty<size_t>(std::move(other)), v{other.v} {
    }
    DegreeProperty(const DegreeProperty&) = delete;

    DegreeProperty& operator=(const DegreeProperty&) = delete;
    DegreeProperty& operator=(DegreeProperty&& other) {
        ComputableProperty<size_t>::operator=(std::move(other));
        v = other.v;
        return *this;
    }
    ~DegreeProperty() = default;
protected:
    Vertex v;

    virtual inline void compute() override final;
};

class _CliquerGraphProperty final : public ComputableProperty<cliquer_graph_t*> {
public:
    _CliquerGraphProperty(const Graph& G):
            ComputableProperty<cliquer_graph_t*>(G) {
    }
    _CliquerGraphProperty(_CliquerGraphProperty&& other):
            ComputableProperty<cliquer_graph_t*>(std::move(other)) {
    }
    _CliquerGraphProperty(const _CliquerGraphProperty&) = delete;

    _CliquerGraphProperty& operator=(const _CliquerGraphProperty&) = delete;
    inline _CliquerGraphProperty& operator=(_CliquerGraphProperty&& other) {
        ComputableProperty<cliquer_graph_t*>::operator=(std::move(other));
        return *this;
    }
    ~_CliquerGraphProperty() {
        clear();
    }
protected:
    virtual inline void compute() override final;

    inline void clear() {
        if(value != nullptr) {
            graph_free(value);
            value = nullptr;
        }
    }
};

/*      *************** Graph ***************      */

/// \brief Wrapper of nauty's graphs
///
/// Graphs generated by nauty's geng/gentreeg utils will be wrapped in a
/// Graph object. For convenience, methods for computing some basic properties
/// of a graph are implemented.
class Graph {
public:
    Graph() = delete;

    /// \brief Copy constructor for geng
    ///
    /// Should only be called by nautypp itself when wrapping a graph object
    /// generated by geng.
    /// \param G Pointer to the nauty object
    /// \param V Number of vertices of the graph
    Graph(const graph* G, size_t V);

    /// \brief Constructor for gentreeg
    ///
    /// Should only be called by nautypp itself when wrapping a tree object
    /// generated by gentreeg
    /// \param parents Vector of parents
    Graph(int* parents, size_t);  // tree construction

    /// \brief Copy/move constructor for geng
    ///
    /// Should only be called by nautypp itself when wrapping a graph object
    /// generated by geng
    /// \param G Pointer to the nauty object
    /// \param V Number of vertices of the graph
    /// \param copy Whether the construction is a copy (default) or a move
    Graph(graph* G, size_t V, bool copy=true);

    /// \brief Constructor of empty graph
    ///
    /// \param V Number of vertices
    Graph(size_t V);

    /// \brief Move constructor
    ///
    /// \param G The graph to move
    Graph(Graph&& G);

    Graph(const Graph&) = delete;


    /// \brief Create a copy of the graph.
    /// \return A copy of the graph.
    inline Graph copy() const {
        return Graph(g, n, true);
    }

    /// \brief Construct the complement of the graph.
    /// \return The complement of this.
    Graph complement() const;

    Graph& operator=(const Graph& other) = delete;
    Graph& operator=(Graph&& other);

    ~Graph() {
        reset();
    }

    /// \brief Number of vertices of the graph
    /// \return The (non-negative) number of vertices of the graph
    inline size_t V() const {
        return n;
    }

    /// \brief Number of edges of the graph
    /// \return The (non-negative) number of edges of the graph
    inline size_t E() const {
        return non_const_self().nb_edges.get();
    }


    /// \brief Get an iterator over all the edges incident to some vertex.
    ///
    /// See Edges.
    /// \param v The vertex to which edges must be incident
    /// \return An iterable over the edges incident to v
    inline Edges edges(Vertex v) const {
        return Edges(*this, v);
    }

    /// \brief Alias to Graph::edges()
    inline Edges edges_incident_to(Vertex v) const {
        return edges(v);
    }

    /// \brief Get an iterator over all the edges of the graph.
    ///
    /// See AllEdges.
    /// \return An iterable over the edges of the graph.
    inline AllEdges edges() const {
        return AllEdges(*this);
    }

    inline explicit operator graph*() {
        return g;
    }

    inline explicit operator const graph*() const {
        return g;
    }

    inline explicit operator cliquer_graph_t*() const {
        return non_const_self()._as_cliquer.get();
    }

    /// \brief Get the degree of a vertex.
    ///
    /// \param v The vertex whose degree is returned
    /// \return The degree of v
    ///
    /// **Example**:
    /// \include degree/degrees.cpp
    inline size_t degree(Vertex v) const {
        if(degrees.size() < V())
            non_const_self().init_degrees();
#ifdef NAUTYPP_DEBUG
        return non_const_self().degrees.at(v).get();
#else
        return non_const_self().degrees[v].get();
#endif
    }

    /// \brief Get the degree of every vertex.
    ///
    /// \return a vector `d` such that `d[v]` is the degree of the vertex `v`.
    ///
    /// **Example**:
    /// \include degree/degrees.cpp
    inline std::vector<size_t> degree() const {
        std::vector<size_t> ret(V());
        for(Vertex v{0}; v < V(); ++v)
            ret[v] = degree(v);
        return ret;
    }

    /// \brief Get the minimum degree of the graph
    ///
    /// \return The minimum degree \f$\delta(G)\f$ of the graph.
    ///
    /// **Example**:
    /// \include degree/degrees.cpp
    inline size_t delta() const {
        auto degrees{degree()};
        return *std::min_element(degrees.cbegin(), degrees.cend());
    }

    /// Alias of delta()
    inline size_t min_degree() const { return delta(); }

    /// \brief Get the maximum degree of the graph
    ///
    /// \return The minimum degree \f$\Delta(G)\f$ of the graph.
    ///
    /// **Example**:
    /// \include degree/degrees.cpp
    inline size_t Delta() const {
        auto degrees{degree()};
        return *std::max_element(degrees.cbegin(), degrees.cend());
    }

    /// Alias of Delta()
    inline size_t max_degree() const { return Delta(); }

    /// \brief Get both the minimum and the maximum degree of the graph.
    ///
    /// See delta() and Delta()
    /// \return A pair `{delta, Delta}`
    ///
    /// **Example**:
    /// \include degree/degrees.cpp
    inline std::pair<size_t, size_t> delta_Delta() {
        auto degrees{degree()};
        auto [min_it, max_it] = std::minmax_element(
            degrees.cbegin(),
            degrees.cend()
        );
        return {*min_it, *max_it};
    }

    /// \brief Alias for the degree distribution of a graph.
    ///
    /// An object of type `DegreeDistribution` is a vector
    /// of pairs `{d, n_d}` where `n_d` is the number of vertices having
    /// degree `d`.
    typedef std::vector<std::pair<size_t, size_t>> DegreeDistribution;

    /// Get the degree distribution of the graph.
    ///
    /// See DegreeDistribution
    ///
    /// **Example**:
    /// \include degree/degrees.cpp
    inline DegreeDistribution degree_distribution() const {
        std::vector<size_t> counts(V(), 0);
        for(Vertex v{0}; v < V(); ++v)
            ++counts[degree(v)];
        std::vector<std::pair<size_t, size_t>> ret;
        for(size_t d{0}; d < V(); ++d) {
            auto count{counts[d]};
            if(count == 0)
                continue;
            ret.emplace_back(d, count);
        }
        return ret;
    }

    /// \brief Determine if two vertices are linked by an edge
    ///
    /// \param v,w Vertices making the edge
    /// \return true if the edge exists and false otherwise
    inline bool are_linked(Vertex v, Vertex w) const {
        return ISELEMENT(
            GRAPHROW(g, v, _m),
            w
        );
    }

    /// Alias of Graph::are_linked()
    inline bool has_edge(Vertex v, Vertex w) const {
        return are_linked(v, w);
    }

    /// \brief Add an edge between to vertices
    ///
    /// \param v,w Vertices to join by an edge
    inline void link(Vertex v, Vertex w) {
        ADDELEMENT(GRAPHROW(g, v, _m), w);
        ADDELEMENT(GRAPHROW(g, w, _m), v);
        degrees[v].set_stale();
        degrees[w].set_stale();
        nb_edges.set_stale();
        _as_cliquer.set_stale();
    }

    /// Alias of link()
    inline void add_edge(Vertex v, Vertex w) {
        return link(v, w);
    }

    /// \brief Get all the neighbours of a vertex
    ///
    /// \param v The vertex whose neighbours are demanded
    inline std::vector<Vertex> neighbours_of(Vertex v) const {
        std::vector<Vertex> ret;
        auto degv{degree(v)};
        if(degv == 0)
            return ret;
        ret.reserve(degv);
        for(auto [_, w] : edges(v))
            ret.push_back(w);
        return ret;
    }

    /// Alias of neighbours_of()
    inline std::vector<Vertex> neighbors_of(Vertex v) const {
        return neighbours_of(v);
    }

    /// \brief Get some neighbour of a vertex.
    ///
    /// **Example**:
    /// \include iterators/neighbours.cpp
    inline Vertex some_neighbour_of(Vertex v) const {
        auto [_, w] = *edges(v).begin();
        return w;
    }

    /// \brief Get a neighbour of some vertex that is different from a specified vertex.
    ///
    /// **Be careful**: no verification is made here!
    /// Therefore if v is a leaf and w is its only neighbour,
    /// the behaviour is undefined!
    /// \param v The vertex whose neighbour is demanded
    /// \param w Some vertex that cannot be the neighbour.
    /// \return A neighbour of v that is not w.
    /// **Example**:
    /// \include iterators/neighbours.cpp
    inline Vertex some_neighbour_of_other_than(Vertex v, Vertex w) const {
        auto it{edges(v).begin()};
        if((*it).second != w)
            return (*it).second;
        else
            return (*++it).second;
    }

    /// \brief Check whether some vertex is a leaf.
    /// \param v The vertex to check.
    /// \return true if \a v is a leaf and false otherwise.
    inline bool is_leaf(Vertex v) const {
        return degree(v) == 1;
    }

    /// \brief Get the leaf-degree of a vertex.
    ///
    /// The leaf-degree of a vertex is the number of its neighbours that
    /// are leaves.
    /// \param v The vertex whose leaf-degree is demanded.
    /// \return The leaf-degree of \a v.
    inline size_t leaf_degree(Vertex v) const {
        size_t ret{0};
        for(auto w : neighbours_of(v))
            if(is_leaf(w))
                ++ret;
        return ret;
    }

    /// \brief Get the leaf-degree of every vertex.
    ///
    /// See degree().
    inline std::vector<size_t> leaf_degree() const {
        std::vector<size_t> ret(V(), 0);
        for(Vertex v{0}; v < V(); ++v)
            if(degree(v) == 1)
                ++ret[first_neighbour_of_nz(v)];
        return ret;
    }

    /// \brief Get the maximal leaf degree of a vertex in the graph.
    ///
    /// See max_degree() or Delta().
    inline size_t max_leaf_degree() const {
        auto leaf_neighbours{leaf_degree()};
        return *std::max_element(
            leaf_neighbours.begin(), leaf_neighbours.end()
        );
    }

    /// \brief Check whether the leaf-degree of every vertex is bounded by some value.
    ///
    /// \param d The upper bound on the leaf-degree.
    /// \return true if `leaf-degree(v) <= d` for every vertex `v`.
    inline bool max_leaf_degree_bounded_by(size_t d) const {
        std::vector<size_t> leaf_neighbours(V(), 0);
        for(Vertex v{0}; v < V(); ++v)
            if(degree(v) == 1)
                if(++leaf_neighbours[first_neighbour_of_nz(v)] > d)
                    return false;
        return true;
    }

    /// Remove all edges incident to some vertex.
    ///
    /// \param v The vertex to isolate.
    inline void isolate_vertex(Vertex v) {
        setword i;
        setword* Nv{setwordof(v)};
        while((i = FIRSTBIT(*Nv)) != WORDSIZE) {
            DELELEMENT(setwordof(i), static_cast<setword>(v));
            DELELEMENT(Nv, i);
            degrees[i].set_stale();
        }
        degrees[v].set_stale();
        nb_edges.set_stale();
        _as_cliquer.set_stale();
    }

    // Cliquer

    /// \brief Find some clique in the graph
    ///
    /// This is basically a wrapper for cliquer::clique_unweighted_find_single
    /// \param minsize A lower bound on the size of the clique.
    /// \param maxsize An upper bound on the size of the clique.
    /// \param maximal Whether or not the clique must be maximal
    /// \return A container of vertices that induce a clique
    Cliquer::Set find_some_clique(size_t minsize, size_t maxsize, bool maximal) const;

    /// \brief Get the clique number of the graph.
    ///
    /// \return The clique number \f$\omega(G)\f$.
    inline size_t max_clique() const {
        return find_some_clique(0, 0, true).size();
    }

    /// \brief Find some independent set in the graph.
    ///
    /// See find_some_clique() and complement().
    /// \return A container of vertices that induce an independent set.
    inline Cliquer::Set find_some_independent_set(size_t minsize, size_t maxsize,
            bool maximal) const {
        return complement().find_some_clique(minsize, maxsize, maximal);
    }

    /// \brief Get the independence number of the graph.
    ///
    /// Simply using \f$\alpha(G) = \omega(G^\complement)\f$.
    /// See Graph::max_clique()
    /// \return The independence number \f$\alpha(G)\f$.
    inline size_t max_independent_set() const {
        return complement().max_clique();
    }

    /// \brief Wrapper for `clique_unweighted_find_all` from `cliquer`.
    ///
    /// Only use if you know how to use cliquer directly!
    /// \param minsize,maxsize,maximal,opts See the documentation from cliquer.
    /// \return The number of generated cliques.
    inline size_t apply_to_cliques(size_t minsize, size_t maxsize, bool maximal,
            clique_options* opts) const {
        return static_cast<size_t>(clique_unweighted_find_all(
            static_cast<cliquer_graph_t*>(*this),
            minsize, maxsize, maximal, opts
        ));
    }

    /// \brief Apply some callback to every generated clique.
    ///
    /// Uses `clique_unweighted_find_all` from `cliquer`.
    /// \param minsize A lower bound on the cliques to generate.
    /// Set to 0 if no lower bound is provided.
    /// \param maxsize An upper bound on the cliques to generate.
    /// If minsize is 0 and maxsize is 0, then all cliques are considered.
    /// \param maximal `true` if only maximal cliques must be considered.
    /// \param callback The function to apply on every clique.
    ///
    /// **Example**:
    /// \include cliquer.cpp
    size_t apply_to_cliques(size_t minsize, size_t maxsize, bool maximal,
            std::function<bool(const Cliquer::Set&)> callback) const;

    /// \brief Apply some callback to every generated clique.
    ///
    /// Uses `clique_unweighted_find_all` from `cliquer`.
    /// \param minsize A lower bound on the cliques to generate.
    /// Set to 0 if no lower bound is provided.
    /// \param maxsize An upper bound on the cliques to generate.
    /// If minsize is 0 and maxsize is 0, then all cliques are considered.
    /// \param maximal `true` if only maximal cliques must be considered.
    /// \param callback The function to apply on every clique.
    ///
    /// **Example**:
    /// \include cliquer.cpp
    size_t apply_to_cliques(size_t minsize, size_t maxsize, bool maximal,
            std::function<bool(const std::vector<Vertex>&)> callback) const;

    /// \brief Generate all cliques from a graph.
    ///
    /// \param minsize,maxsize,maximal See find_some_clique.
    /// \param A container of cliques.
    ///
    /// **Example**:
    /// \include cliquer.cpp
    std::vector<Cliquer::Set> get_all_cliques(size_t minsize, size_t maxsize,
            bool maximal) const;

    friend class EdgeIterator;
    friend class AllEdgeIterator;
    friend class NautyContainer;
    friend class EdgeProperty;
    friend class DegreeProperty;

/* ******************** Static Methods ******************** */

/*      *************** Creation of Graph From Other Representations ***************      */

    /// \brief Construct a Graph from a row-flattened triangular adjacency matrix.
    ///
    /// \param A The flattened adjacency matrix.
    /// \param upper true if \a A represents an upper triangular matrix and false otherwise.
    template <typename T>
    inline static Graph from_adjacency_matrix(
            const std::vector<T> A, bool upper=true) {
        auto V{static_cast<size_t>(
            std::sqrt(static_cast<float>(A.size())) + .5
        )};
        return from_adjacency_matrix(A, V, upper);
    }

    /// \brief Construct a Graph from a row-flattened triangular adjacency matrix.
    ///
    /// \param A The flattened adjacency matrix.
    /// \param V the number of vertices of V.
    /// \param upper true if \a A represents an upper triangular matrix and false otherwise.
    template <typename T>
    inline static Graph from_adjacency_matrix(
            const std::vector<T> A,
            size_t V, bool upper=true) {
        if(upper)
            return from_adjacency_matrix_upper(A, V);
        else
            return from_adjacency_matrix_lower(A, V);
    }

    template <typename T>
    static Graph from_adjacency_matrix_upper(const std::vector<T> A, size_t V) {
        if(V*V != A.size())
            throw std::runtime_error("Wrong dimensions for adjacency matrix");
        Graph ret(V);
        for(Vertex v{0}; v+1 < V; ++v)
            for(Vertex w{v+1}; w < V; ++V)
                if(A.at(v*V + w))
                    ret.link(v, w);
        return ret;
    }

    template <typename T>
    static Graph from_adjacency_matrix_lower(const std::vector<T> A, size_t V) {
        if(V*V != A.size())
            throw std::runtime_error("Wrong dimensions for adjacency matrix");
        Graph ret(V);
        for(Vertex v{1}; v < V; ++v)
            for(Vertex w{0}; w < v; ++w)
                if(A.at(v*V + w))
                    ret.link(v, w);
        return ret;
    }

/*      *************** Construction of Graph Families ***************      */

    /// Create a path graph.
    ///
    /// \param N The number of vertices.
    /// \return The graph \f$P_N\f$.
    inline static Graph make_path(size_t N) {
        Graph ret(N);
        for(Vertex v{1}; v < N; ++v)
            ret.link(v-1, v);
        return ret;
    }

    /// Create a cycle graph.
    ///
    /// \param N The number of vertices.
    /// \return The graph \f$C_N\f$.
    inline static Graph make_cycle(size_t N) {
        auto ret{make_path(N)};
        ret.link(0, N-1);
        return ret;
    }

    /// Create a claw graph.
    ///
    /// \param N The number of leaves.
    /// \return The graph \f$K_{1,N}\f$.
    inline static Graph make_claw(size_t N) {
        Graph ret(N+1);
        for(Vertex v{0}; v < N; ++v)
            ret.link(v, N);
        return ret;
    }

    /// Alias of make_claw()
    inline static Graph make_star(size_t N) {
        return make_claw(N);
    }

    /// Create a complete graph.
    ///
    /// \param N The number of vertices.
    /// \return The graph \f$K_N\f$.
    inline static Graph make_complete(size_t N) {
        Graph ret(N);
        for(Vertex v{0}; v < N; ++v)
            for(Vertex w{v+1}; w < N; ++w)
                ret.link(v, w);
        return ret;
    }

    /* return K_{s,t} */
    /// Create a complete bipartite graph.
    ///
    /// \param s,t The number of vertices in each part.
    /// \return The graph \f$K_{s,t}\f$.
    inline static Graph make_complete_bipartite(size_t s, size_t t) {
        Graph ret(s+t);
        for(Vertex v{0}; v < s; ++v)
            for(Vertex w{s}; w < s+t; ++w)
                ret.link(v, w);
        return ret;
    }
private:
    size_t n;
    bool host;
#ifdef NAUTYPP_SGO
    graph __small_graph_buffer[NAUTYPP_SMALL_GRAPH_SIZE];
#endif
    graph* g;

    static inline constexpr size_t _m{1};

    EdgeProperty nb_edges;
    std::vector<DegreeProperty> degrees;
    _CliquerGraphProperty _as_cliquer;

    inline void reset() {
        if(not host or g == nullptr)
            return;
        delete[] g;
        g = nullptr;
        host = false;
    }

    inline void assign_from(const graph* G, size_t V) {
#ifdef NAUTYPP_SGO
        if(V <= NAUTYPP_SMALL_GRAPH_SIZE) {
            g = __small_graph_buffer;
            host = false;
        } else {
            g = new graph[_m*V];
            host = true;
        };
#else
        g = new graph[_m*V];
        host = true;
#endif
        memcpy(g, G, _m*V*sizeof(*G));
    }

    inline void init_degrees() {
        degrees.reserve(n);
        for(size_t v{degrees.size()}; v < V(); ++v)
            degrees.emplace_back(*this, v);
    }

    inline size_t __get_m() const {
        return _m;
    }

    // important for the read-only graph properties
    inline Graph& non_const_self() const {
        return *const_cast<Graph*>(this);
    }

    inline Vertex first_neighbour_of(Vertex v) const {
        setword Nv{*setwordof(v)};
        Vertex ret{static_cast<Vertex>(FIRSTBIT(Nv))};
        if(ret == WORDSIZE)
            throw std::runtime_error("Vertex has no neighbour");
        return ret;
    }

    inline Vertex first_neighbour_of_nz(Vertex v) const {
        return static_cast<Vertex>(FIRSTBITNZ(*GRAPHROW(g, v, _m)));
    }

    static inline size_t m_from_n(size_t n) {
        return 1 + (n-1) / WORDSIZE;
    }

    inline setword* setwordof(Vertex v) {
        return GRAPHROW(g, v, _m);
    }

    inline const setword* setwordof(Vertex v) const {
        return GRAPHROW(g, v, _m);
    }
};

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

/* ******************** Multithreading Tools ******************** */

/// \brief Status of the internal graph buffer.
enum class NautyStatus {
    DATA_AVAILABLE,  ///< Buffer is still active.
    END_OF_THREAD    ///< Buffer is deactivated.
};

// forward declarations
class Nauty;
template <GraphFunctionType Callback>
class NautyWorker;

/*      *************** Containers ***************      */

/// \brief Communication buffer between producer and consumer threads.
///
/// The buffer is separated into a *read* buffer and a *write* buffer.
/// The consumer thread pops from the former, while the producer thread inserts in the latter.
/// When the read buffer is empty, the consumer will swap the two buffers.
template <typename T>
class ContainerBuffer {
public:
    ContainerBuffer() = delete;
    ContainerBuffer(size_t maxsize):
            _size{maxsize},
            _read_buffer(), _write_buffer(),
            _writable{true}, _should_swap{false} {
        _read_buffer.reserve(maxsize);
        _write_buffer.reserve(maxsize);
    }

    ContainerBuffer(const ContainerBuffer&) = delete;
    ContainerBuffer(ContainerBuffer&&) noexcept = default;

    ~ContainerBuffer() {
        if(_read_buffer.size() > 0)
            std::cerr << "Destroying a buffer w/ reading capability";
        if(_write_buffer.size() > 0)
            std::cerr << "Missed a swap (" << _write_buffer.size() << ")\n";
    }

    /// \brief Tries to move something into the write buffer.
    ///
    /// The insertion can only succeed if the buffer is not full.
    /// **Be careful**: if the insertion succeeds, the object is *moved* and not *copied*!
    /// \param G The element to insert.
    /// \return true if the element was inserted.
    inline bool push(T& G) {
        if(_should_swap or write_size() == _size) {
            _should_swap = true;
            return false;
        }
        _write_buffer.push_back(std::move(G));
        return true;
    }

    class EmptyBuffer : public std::exception {
    };

    /// \brief Extract an element from the read buffer.
    ///
    /// The buffers can be swapped if the read buffer is empty
    /// and the write buffer is full.
    /// \throws EmptyBuffer if both the read and write buffers are
    /// empty, and the buffer is deactivated.
    /// \return An element from the read buffer.
    inline T pop() {
        do {
            __verify_not_empty_read();
        } while(_read_buffer.empty() and _writable);
        if(_read_buffer.empty()) {
            if(write_size() > 0)
                std::cerr << "SHOULD NOT THROW\n";
            throw EmptyBuffer();
        }
        T ret{std::move(_read_buffer.back())};
        _read_buffer.pop_back();
        return ret;
    }

    /// \brief Determine whether or not the buffer is active.
    ///
    /// \return true if the buffer is active and false otherwise.
    inline bool writable() const {
        return _writable;
    }

    /// \brief Enable the buffer.
    inline void enable_write() {
        _writable = true;
    }

    // \brief Disable the buffer
    inline void disable_write() {
        _should_swap = true;
        _writable = false;
    }
private:
    size_t                    _size;
    std::vector<T>            _read_buffer;
    std::vector<T>            _write_buffer;
    volatile std::atomic_bool _writable;
    volatile std::atomic_bool _should_swap;

    inline void __verify_not_empty_read() {
        using namespace std::chrono_literals;
        if(read_size() > 0) {
            return;
        } else {
            while(_writable and not _should_swap)
                std::this_thread::sleep_for(10ms);
            if(_should_swap)
                std::swap(_read_buffer, _write_buffer);
            _should_swap = false;
        }
    }

    friend class BaseNautyWorker;
    template <GraphFunctionType Callback>
    friend class NautyWorker;

    /* NO LOCK IS PERFORMED HERE! */
    inline size_t read_size() const {
        return _read_buffer.size();
    }

    /* NO LOCK IS PERFORMED HERE! */
    inline size_t write_size() const {
        return _write_buffer.size();
    }
};

/// Alias for a producer-consumer buffer containing graphs.
/// See Graph.
typedef ContainerBuffer<Graph> NautyContainerBuffer;


/// \brief Container of multiple inter-thread buffers.
///
/// See NautyContainerBuffer.
class NautyContainer {
public:
    NautyContainer(): _worker_buffers() {
    }

    inline std::shared_ptr<NautyContainerBuffer> add_new_buffer(size_t buffer_size) {
        _worker_buffers.push_back(
            std::make_shared<NautyContainerBuffer>(buffer_size)
        );
        return _worker_buffers.back();
    }

    template <typename... Args>
    inline void emplace(Args&&... args) {
        Graph G(std::forward<Args>(args)...);
        while(true) {
            for(auto& buffer : _worker_buffers) {
                if(buffer->writable() and buffer->push(G))
                    return;
            }
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(1ms);
        }
    }

    inline void _add_gentree_tree(int* parents, size_t n) {
        emplace(parents, n);
    }

    friend class Nauty;
private:
    typedef std::vector<std::shared_ptr<NautyContainerBuffer>> ContainerVector;
    ContainerVector _worker_buffers;

    inline void set_over() {
        for(auto& buffer : _worker_buffers)
            buffer->disable_write();
    }
};

/*      *************** Workers ***************      */

class BaseNautyWorker {
public:
    BaseNautyWorker(std::shared_ptr<NautyContainerBuffer> buffer):
            _buffer{std::move(buffer)} {
    }

    ~BaseNautyWorker() = default;

    void run() {
        while(true) {
            try {
                Graph G{_buffer->pop()};
                (*this)(G);
            } catch(NautyContainerBuffer::EmptyBuffer&) {
                break;
            }
        }
    }

    virtual void operator()(Graph& G) = 0;
protected:
    typedef std::shared_ptr<NautyContainerBuffer> BufferPtr;
    BufferPtr  _buffer;
};

template <GraphFunctionType GraphFunction>
class NautyWorker final : public BaseNautyWorker {
public:
    typedef GraphFunction callback_t;

    NautyWorker(std::shared_ptr<NautyContainerBuffer> buffer, callback_t callback):
            BaseNautyWorker(buffer), _callback{callback} {
    }

    NautyWorker(NautyWorker&) = delete;
    NautyWorker(NautyWorker&&) = default;

#ifdef NAUTYPP_DEBUG
    ~NautyWorker() noexcept(false) {
        if(_buffer->writable())
            throw std::runtime_error("Destroying a worker with a readable buffer.");
        if(_buffer->read_size() > 0)
            std::cerr << "Destroying worker with read buffer size "
                      << _buffer->read_size() << std::endl;
    }
#else
    ~NautyWorker() = default;
#endif

    virtual void operator()(Graph& G) override final {
        _callback(G);
    }
protected:
    callback_t _callback;
};

template <GraphRefFunctionType Callback>
class NautyWorkerWrapper final : public BaseNautyWorker {
public:
    NautyWorkerWrapper(std::shared_ptr<NautyContainerBuffer> buffer):
            BaseNautyWorker(buffer), _callback() {
    }

    virtual void operator()(Graph& G) override final {
        _callback(G);
    }

    inline void join(NautyWorkerWrapper& other) {
        _callback.join(std::move(other._callback));
    }

    inline const typename Callback::ResultType& get() const& {
        return _callback.get();
    }

    inline typename Callback::ResultType&& get() && {
        return _callback.get();
    }
private:
    Callback _callback;
};

/*      *************** Nauty ***************      */

/// \brief Wrapper for geng/gentreeg
class Nauty {
public:
    /// \brief Constructor of Nauty.
    ///
    /// See NautyParameters.
    Nauty(const NautyParameters& params):
            parameters{params} {
        if(parameters.connected)
            parameters.min_deg = std::max(1, parameters.min_deg);
        parameters.max_deg = std::min(parameters.Vmax-1, parameters.max_deg);
        if(parameters.Vmax < parameters.V)
            parameters.Vmax = parameters.V;
    }

    /// \brief Run some callback on all graphs generated by geng/gentreeg.
    ///
    /// \param callback The function to execute on every graph.
    /// \param nb_workers The number of threads to create to dispatch the generated graphs.
    /// \param worker_buffer_size The buffer size for every worker.
    ///
    /// **Example**:
    /// \include multithreaded/count_triangle_free_graphs.cpp
    template <GraphFunctionType GraphFunction>
    void run_async(GraphFunction callback,
            size_t nb_workers=std::thread::hardware_concurrency(),
            size_t worker_buffer_size=5'000) {
        Nauty::reset_container();
        std::vector<std::thread> worker_threads;
        std::vector<NautyWorker<GraphFunction>> workers;
        worker_threads.reserve(nb_workers);
        workers.reserve(nb_workers);
        static char name_buffer[32];
        for(size_t i{0}; i < nb_workers; ++i) {
            auto worker_buffer{
                Nauty::_container->add_new_buffer(worker_buffer_size)
            };
            workers.emplace_back(worker_buffer, callback);
            worker_threads.emplace_back(
                &NautyWorker<GraphFunction>::run,
                &workers.back()
            );
            std::sprintf(name_buffer, "Worker %u", static_cast<unsigned>(i+1));
            rename_thread(worker_threads.back(), name_buffer);
        }
        auto geng_thread{start_nauty()};
        geng_thread.join();
        for(size_t i{0}; i < nb_workers; ++i)
            worker_threads.at(i).join();
    }

    /// Alternative version of run_async.
    ///
    /// Here, the callback is not provided by reference but by type.
    ///
    /// The template type \a Callback must:
    /// - provide a type `Callback::ResultType`
    /// - define `operator()(const Graph&)`
    /// - define `join(Callback&& other)`
    /// - define `ResultType&& get()`
    ///
    /// **Example**:
    /// \include multithreaded/heavy_callback.cpp
    template <GraphRefFunctionType Callback>
    auto run_async(
            size_t nb_workers=std::thread::hardware_concurrency(),
            size_t worker_buffer_size=5'000) -> typename Callback::ResultType {
        Nauty::reset_container();
        std::vector<NautyWorkerWrapper<Callback>> wrappers;
        std::vector<std::thread> workers;
        wrappers.reserve(nb_workers);
        workers.reserve(nb_workers);
        static char name_buffer[32];
        for(size_t i{0}; i < nb_workers; ++i) {
            auto worker_buffer{Nauty::_container->add_new_buffer(worker_buffer_size)};
            wrappers.emplace_back(worker_buffer);
            workers.emplace_back(
                &NautyWorkerWrapper<Callback>::run,
                std::addressof(wrappers.back())
            );
            std::sprintf(name_buffer, "Worker %u", static_cast<unsigned>(i+1));
            rename_thread(workers.back(), name_buffer);
        }
        auto geng_thread{start_nauty()};
        geng_thread.join();
        for(size_t i{0}; i < nb_workers; ++i)
            workers.at(i).join();
        for(size_t idx{1}; idx < nb_workers; ++idx)
            wrappers.at(0).join(wrappers.at(idx));
        return static_cast<NautyWorkerWrapper<Callback>&&>(wrappers.at(0)).get();
    }

    static inline std::unique_ptr<NautyContainer>& get_container() {
#ifdef NAUTYPP_DEBUG
        if(not Nauty::_container)
            throw std::runtime_error("No initialized container");
#endif
        return Nauty::_container;
    }

private:
    NautyParameters parameters;

    inline std::thread start_nauty() {
        std::thread ret{
            parameters.tree
            ? start_gentreeg()
            : start_geng()
        };
        rename_thread(
            ret,
            parameters.tree
            ? "nauty-gentreeg"
            : "nauty-geng"
        );
        return ret;
    }

    inline std::thread start_gentreeg() {
        strcpy(params[0], "gentreeg");
        std::sprintf(
            params[1],
            "-D%d%s",
            parameters.max_deg,
#ifdef NAUTYPP_DEBUG
            ""
#else
            "q"
#endif
        );
        std::sprintf(
            params[2], "%d", parameters.V
        );
        for(auto i{0}; i < Nauty::GENG_ARGC; ++i)
            geng_argv[i] = params[i];
        std::thread t(
            [this]() {
                _gentreeg_main(Nauty::GENG_ARGC, geng_argv);
                this->set_container_over();
            }
        );
        return t;
    }

    inline std::thread start_geng() {
        strcpy(params[0], "geng");
        for(auto i{0}; i < Nauty::GENG_ARGC; ++i)
            geng_argv[i] = params[i];
        std::thread t(
            [this]() {
                int min_deg, max_deg;
                for(int V{this->parameters.V}; V <= this->parameters.Vmax; ++V) {
                    auto& parameters{this->parameters};
                    min_deg = std::min(parameters.min_deg, V-1);
                    max_deg = std::min(parameters.max_deg, V-1);
                    std::sprintf(
                        params[1],
                        "-%s%s%s%s%s%s%s%s%s%s%sd%dD%d%s",
                        parameters.connected     ? "c" : "",
                        parameters.biconnected   ? "C" : "",
                        parameters.triangle_free ? "t" : "",
                        parameters.C4_free       ? "f" : "",
                        parameters.C5_free       ? "p" : "",
                        parameters.K4_free       ? "k" : "",
                        parameters.chordal       ? "T" : "",
                        parameters.split         ? "S" : "",
                        parameters.perfect       ? "P" : "",
                        parameters.claw_free     ? "F" : "",
                        parameters.bipartite     ? "b" : "",
                        min_deg,
                        max_deg,
#ifdef NAUTYPP_DEBUG
                        ""
#else
                        "q"
#endif
                    );
                    std::sprintf(this->geng_argv[2], "%d", V);
                    _geng_main(Nauty::GENG_ARGC, this->geng_argv);
                }
                this->set_container_over();
            }
        );
        return t;
    }

    static inline constexpr auto GENG_ARGV_BUFFER_SIZE{64};
    static inline constexpr auto GENG_ARGC{3};
    char params[Nauty::GENG_ARGC][Nauty::GENG_ARGV_BUFFER_SIZE];
    char* geng_argv[Nauty::GENG_ARGC];

    static void reset_container() {
        Nauty::_container.reset(new NautyContainer());
    }

    static std::unique_ptr<NautyContainer> _container;
    inline void set_container_over() {
        Nauty::get_container()->set_over();
    }
};
}  // namespace nautypp

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

#endif
