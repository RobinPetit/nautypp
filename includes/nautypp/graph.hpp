#ifndef NAUTYPP_GRAPH_HPP
#define NAUTYPP_GRAPH_HPP

#include <algorithm>
#include <cmath>
#include <functional>

#include "nauty/planarity.h"

#include <nautypp/algorithms.hpp>
#include <nautypp/aliases.hpp>
#include <nautypp/cliquer.hpp>
#include <nautypp/iterators.hpp>
#include <nautypp/properties.hpp>

namespace nautypp {
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
    inline Neighbours neighbours_of(Vertex v) const {
        return {*this, v};
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

    /// \brief Create a new graph corresponding to the vertex-disjoint union with another graph.
    ///
    /// \param other The other graph in the disjoint union
    /// \return The union of `*this` and `other`.
    inline Graph disjoint_union(const Graph& other) const {
        return Graph::disjoint_union(*this, other);
    }

    static Graph disjoint_union(const Graph& G1, const Graph& G2);

    /// \brief count the number of connected components.
    ///
    /// \return The number of conneced components of the graph.
    inline size_t nb_connected_components() const {
        return ConnectedComponents(*this).get_nb_components();
    }

    /// Get the connected components of the graph.
    ///
    /// See ConnectedComponents
    inline ConnectedComponents get_connected_components() const {
        return ConnectedComponents(*this);
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

    /// Determine whether a graph is planar or not.
    ///
    /// Basically a wrapper for nauty::planarity::sparseg_adjl_is_planar.
    /// Be aware that this function call must create an intermediate
    /// representation for compatibility, this might be slow!
    /// \return `true` if the graph is planar and `false` otherwise.
    ///
    /// **Example**:
    /// \include planar.cpp
    inline bool is_planar() const {
        const auto n{this->V()};
        // see nauty::planarg.c::isplanar
        DYNALLSTAT(t_ver_sparse_rep,  V, V_sz);
        DYNALLSTAT(t_adjl_sparse_rep, A, A_sz);
        DYNALLOC1(t_ver_sparse_rep,   V, V_sz, n,       "is_planar");
        DYNALLOC1(t_adjl_sparse_rep,  A, A_sz, 2*E()+1, "is_planar");
        t_dlcl **dfs_tree, **back_edges, **mult_edges;
        t_ver_edge *embed_graph;
        int edge_pos, v, w, c;
        // Conversion to nauty::planarity adjl format
        int k{0};
        for(Vertex v{0}; v < n; ++v) {
            if(degree(v) == 0) {
                V[v].first_edge = NIL;
            } else {
                V[v].first_edge = k;
                for(Vertex w : neighbours_of(v)) {
                    A[k].end_vertex = w;
                    A[k].next = k+1;
                    ++k;
                    // No need to handle loop
                }
                A[k-1].next = NIL;
            }
        }
        // see nauty::planarity.c
        bool ret{static_cast<bool>(sparseg_adjl_is_planar(
            V, n, A, &c,
            &dfs_tree, &back_edges, &mult_edges,
            &embed_graph, &edge_pos, &v, &w
        ))};
        sparseg_dlcl_delete(dfs_tree, n);
        sparseg_dlcl_delete(back_edges, n);
        sparseg_dlcl_delete(mult_edges, n);
        embedg_VES_delete(embed_graph, n);
        DYNFREE(V, V_sz);
        DYNFREE(A, A_sz);
        return ret;
    }

    /// See is_planar
    static inline bool is_planar(const Graph& G) {
        return G.is_planar();
    }

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
    size_t _m{1};
    graph* g;

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
}

#endif
