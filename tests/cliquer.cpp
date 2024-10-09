#include <catch2/catch.hpp>

#include <nautypp/nauty.hpp>

using namespace nautypp;

static inline uint64_t binom2(uint64_t n) {
    return (n*(n-1)) / 2;
}

static inline size_t count_cliques(const Graph& G, size_t minsize, size_t maxsize, bool maximal) {
    size_t count{0};
    G.apply_to_cliques(
        minsize, maxsize, maximal,
        [&count](const Cliquer::Set&) {
            ++count;
            return true;
        }
    );
    return count;
}

TEST_CASE("cliquer conversion") {
    constexpr size_t n{10};
    auto G{Graph::make_complete(n)};
    cliquer_graph_t* as_cliquer_ptr{G};
    auto nb_edges{static_cast<size_t>(graph_edge_count(as_cliquer_ptr))};
    REQUIRE(nb_edges == binom2(n));
    for(Vertex v{0}; v < G.V(); ++v) {
        REQUIRE(graph_vertex_degree(as_cliquer_ptr, v) == n-1);
    }
}

TEST_CASE("cliquer conversion disconnected") {
    constexpr size_t n{10};
    auto G{
        Graph::disjoint_union(
            Graph::make_complete(n),
            Graph::make_complete(n)
        )
    };
    cliquer_graph_t* as_cliquer_ptr{G};
    auto nb_edges{static_cast<size_t>(graph_edge_count(as_cliquer_ptr))};
    REQUIRE(nb_edges == 2*binom2(n));
    for(Vertex v{0}; v < G.V(); ++v) {
        REQUIRE(graph_vertex_degree(as_cliquer_ptr, v) == n-1);
    }
}

TEST_CASE("Count cliques in Kn") {
    size_t n = GENERATE(range(3, 10));

    auto Kn{Graph::make_complete(n)};
    size_t expected_count{(1 << n) - 1u};
    REQUIRE(Kn.get_all_cliques(1, n, false).size() == expected_count);
    REQUIRE(count_cliques(Kn, 1, n, false) == expected_count);

    REQUIRE(count_cliques(Kn, 1, n, true) == 1);

    REQUIRE(Kn.max_clique() == n);
    REQUIRE(Kn.max_independent_set() == 1);
}

TEST_CASE("Count cliques in Kn \\cup Kn") {
    size_t n = GENERATE(range(3, 10));

    auto Kn{Graph::make_complete(n)};
    auto G{Graph::disjoint_union(Kn, Kn)};
    size_t expected_count{(1 << n) - 1u};
    REQUIRE(G.get_all_cliques(1, n, false).size() == 2*expected_count);
    REQUIRE(count_cliques(G, 1, n, false) == 2*expected_count);

    REQUIRE(count_cliques(G, 1, n, true) == 2);

    REQUIRE(G.max_clique() == n);
    REQUIRE(G.max_independent_set() == 2);
}
