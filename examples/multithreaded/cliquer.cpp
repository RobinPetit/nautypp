#include <atomic>
#include <iostream>

#include <nautypp/nauty.hpp>

using namespace nautypp;

/*
 * Test for mixing multithreaded Nauty::run_async and Graph::apply_to_cliques.
 * To make sure that everything goes alright with cliquer.
 */

int main() {
    NautyParameters params{
        .tree=false,
        .connected=false,
        .V=3,
        .Vmax=10
    };
    Nauty nauty(params);
    std::atomic_uint32_t maxclique{0};
    nauty.run_async(
        [&maxclique](const Graph& G) {
            G.apply_to_cliques(1, G.V(), true,
                [&maxclique](const Cliquer::Set& s) -> boolean {
                    auto size{s.size()};
                    if(size > maxclique)
                        maxclique = size;
                    return true;
                }
            );
        },
        16
    );
    std::cout << "The biggest clique found contained " << maxclique << " vertices\n";
    return 0;
}
