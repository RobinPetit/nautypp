#include <atomic>

#include <catch2/catch.hpp>

#include <nautypp/nauty.hpp>

using namespace nautypp;

static inline size_t count_graphs(const NautyParameters& params) {
    std::atomic_size_t count{0};
    Nauty().run_async(
        [&count](const Graph&) {
            ++count;
        },
        params
    );
    return count;
}

TEST_CASE("Count generated default graphs") {
/*
$ for n in `seq 1 10`; do geng -u $n; done
>A geng -d0D0 n=1 e=0
>Z 1 graphs generated in 0.00 sec
>A geng -d0D1 n=2 e=0-1
>Z 2 graphs generated in 0.00 sec
>A geng -d0D2 n=3 e=0-3
>Z 4 graphs generated in 0.00 sec
>A geng -d0D3 n=4 e=0-6
>Z 11 graphs generated in 0.00 sec
>A geng -d0D4 n=5 e=0-10
>Z 34 graphs generated in 0.00 sec
>A geng -d0D5 n=6 e=0-15
>Z 156 graphs generated in 0.00 sec
>A geng -d0D6 n=7 e=0-21
>Z 1044 graphs generated in 0.00 sec
>A geng -d0D7 n=8 e=0-28
>Z 12346 graphs generated in 0.01 sec
>A geng -d0D8 n=9 e=0-36
>Z 274668 graphs generated in 0.06 sec
>A geng -d0D9 n=10 e=0-45
>Z 12005168 graphs generated in 1.93 se
*/
    static std::vector<size_t> ns{
        {1, 2, 3, 4, 5, 6, 7, 8, 9}
    };
    static std::vector<size_t> counts{
        {1, 2, 4, 11, 34, 156, 1'044, 12'346, 274'668}
    };
    auto i = GENERATE(range(0, 9));
    auto n = ns[i];
    auto expected_count = counts[i];
    NautyParameters params{
        .connected=false,
        .V=static_cast<int>(n), .Vmax=static_cast<int>(n)
    };
    REQUIRE(count_graphs(params) == expected_count);
}

TEST_CASE("Count Biconnected triangle-free graphs") {
/*
$ for n in `seq 3 10`; do ./geng -uCt $n; done
>A ./geng -Ctd2D2 n=3 e=3
>Z 0 graphs generated in 0.00 sec
>A ./geng -Ctd2D3 n=4 e=4
>Z 1 graphs generated in 0.00 sec
>A ./geng -Ctd2D4 n=5 e=5-6
>Z 2 graphs generated in 0.00 sec
>A ./geng -Ctd2D5 n=6 e=6-9
>Z 6 graphs generated in 0.00 sec
>A ./geng -Ctd2D6 n=7 e=7-12
>Z 16 graphs generated in 0.00 sec
>A ./geng -Ctd2D7 n=8 e=8-16
>Z 78 graphs generated in 0.00 sec
>A ./geng -Ctd2D8 n=9 e=9-20
>Z 415 graphs generated in 0.00 sec
>A ./geng -Ctd2D9 n=10 e=10-25
>Z 3374 graphs generated in 0.01 sec
*/
    std::vector<size_t> ns{
        {3, 4, 5, 6, 7, 8, 9, 10}
    };
    std::vector<size_t> counts{
        {0, 1, 2, 6, 16, 78, 415, 3374}
    };
    auto i = GENERATE(range(0, 7));
    auto n{ns[i]};
    auto expected_count{counts[i]};

    NautyParameters params{
        .biconnected=true,
        .triangle_free=true,
        .V=static_cast<int>(n),
        .Vmax=static_cast<int>(n)
    };
    REQUIRE(count_graphs(params) == expected_count);
}

TEST_CASE("Read graph6") {
    Nauty nauty;
    std::atomic_int count{0};
    nauty.run_async(
        [&count](const Graph& graph) {
            ++count;
            (void)graph;
        },
        "geng_4_biconnected.graph6",  // path
        4  // max graph size
    );
/*
$ geng -C 4 -u
>A geng -Cd2D3 n=4 e=4-6
>Z 3 graphs generated in 0.00 sec
*/
    REQUIRE(count == 3);
}

TEST_CASE("Read sparse6") {
    Nauty nauty;
    std::atomic_int count{0};
    nauty.run_async(
        [&count](const Graph& graph) {
            ++count;
            (void)graph;
        },
        "geng_4_biconnected.sparse6",  // path
        4  // max graph size
    );
/*
$ geng -C 4 -u
>A geng -Cd2D3 n=4 e=4-6
>Z 3 graphs generated in 0.00 sec
*/
    REQUIRE(count == 3);
}
