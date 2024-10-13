#include <algorithm>

#include <catch2/catch.hpp>

#include <nautypp/nautypp>

static inline uint64_t binom2(uint64_t n) {
    return (n*(n-1)) / 2;
}

template <typename T>
static inline bool contains(const std::vector<T>& v, const T& x) {
    return std::find(v.begin(), v.end(), x) != v.end();
}

using namespace nautypp;

TEST_CASE("empty graph is empty") {
    constexpr size_t V{5};
    Graph G(V);
    REQUIRE(G.V() == V);
    REQUIRE(G.E() == 0);

    size_t nb_edges{0};
    for(auto _ : G.edges())
        ++nb_edges;
    REQUIRE(nb_edges == 0);

    nb_edges = 0;
    for(Vertex v{0}; v < G.V(); ++v)
        for(Vertex _ : G.neighbors_of(v))
            ++nb_edges;
    REQUIRE(nb_edges == 0);

    for(Vertex v{0}; v < G.V(); ++v)
        REQUIRE(G.degree(v) == 0);
}

TEST_CASE("Complete graph is complete") {
    size_t n = GENERATE(range(1, 10));
    auto Kn{Graph::make_complete(n)};
    REQUIRE(Kn.V() == n);
    REQUIRE(Kn.E() == binom2(n));
    for(Vertex v{0}; v < Kn.V(); ++v) {
        REQUIRE(Kn.degree(v) == n-1);
        std::vector<Vertex> Nv{Kn.neighbours_of(v)};
        REQUIRE(Nv.size() == n-1);
        for(Vertex w{0}; w < Kn.V(); ++w) {
            if(w == v)
                continue;
            REQUIRE(contains(Nv, w));
        }
        REQUIRE(not contains(Nv, v));
    }
}

TEST_CASE("Complete bipartite is complete bipartite") {
    size_t s = GENERATE(range(1, 6));
    size_t t = GENERATE(range(1, 6));
    auto Kst{Graph::make_complete_bipartite(s, t)};

    std::vector<Vertex> first_part;
    first_part.reserve(s);
    for(size_t v{0}; v < s; ++v)
        first_part.push_back(v);
    std::vector<Vertex> second_part;
    for(size_t v{0}; v < t; ++v)
        second_part.push_back(s+v);
    second_part.reserve(t);

    REQUIRE(Kst.V() == s+t);
    REQUIRE(Kst.E() == s*t);

    for(Vertex v{0}; v < Kst.V(); ++v) {
        std::vector<Vertex> Nv{Kst.neighbours_of(v)};
        auto& expected{
            contains(first_part, v) ? second_part : first_part
        };
        REQUIRE(Nv == expected);
    }
}

TEST_CASE("Disjoint union") {
    constexpr size_t n{5};
    auto G1{Graph::make_complete_bipartite(n, n)};
    auto G2{Graph::make_complete(n)};
    auto G{Graph::disjoint_union(G1, G2)};
    REQUIRE(G.V() == 15);
    REQUIRE(G.E() == G1.E() + G2.E());
    REQUIRE(G.nb_connected_components() == 2);

    std::vector<Vertex> first_component;
    std::vector<Vertex> second_component;
    for(size_t v{0}; v < n; ++v) {
        first_component.push_back(2*v + 0);
        first_component.push_back(2*v + 1);
        second_component.push_back(2*n + v);
    }
    auto components{G.get_connected_components()};
    REQUIRE(components.get_component_of(0) == first_component);
    REQUIRE(components.get_component_of(G.V()-1) == second_component);
}

TEST_CASE("Big graphs") {
    auto k = GENERATE(values({6, 8, 10, 12, 14}));
    auto n{1ull << (k-1)};
    auto K2n{Graph::make_complete(n)};  // K_{2^n}
    REQUIRE(K2n.V() == n);
    REQUIRE(K2n.E() == binom2(n));

    std::vector<Vertex> expected_neighbours;
    expected_neighbours.reserve(n-1);
    for(Vertex v{0}; v < n; ++v) {
        expected_neighbours.clear();
        for(Vertex w{0}; w < n; ++w)
            if(w != v)
                expected_neighbours.push_back(w);
        std::vector<Vertex> Nv{K2n.neighbours_of(v)};
        REQUIRE(expected_neighbours == Nv);
    }
}
