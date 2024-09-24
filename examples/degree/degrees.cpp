#include <iostream>

#include <nautypp/nauty.hpp>

using namespace nautypp;

int main() {
    Graph K_1_5{Graph::make_claw(5)};
    size_t min_degree{K_1_5.delta()};
    size_t max_degree{K_1_5.Delta()};
    std::cout << "K_{1,5} has min degree "
              << min_degree << " and max degree "
              << max_degree << std::endl;
    auto [mindeg, maxdeg] = K_1_5.delta_Delta();
    std::cout << "K_{1,5} has min degree "
              << mindeg << " and max degree "
              << maxdeg << std::endl;
    std::cout << "Vertex 0 has degree " << K_1_5.degree(0) << std::endl;
    std::cout << "Vertex 5 has degree " << K_1_5.degree(5) << std::endl;
    std::cout << "Degree distribution of K_{1,5}:" << std::endl;
    for(auto [d, nd] : K_1_5.degree_distribution())
        std::cout << "  " << nd << " vertices of degree " << d << std::endl;
    auto degrees{K_1_5.degree()};
    for(size_t v{0}; v < K_1_5.V(); ++v)
        std::cout << "deg(" << v << ") = " << degrees[v] << std::endl;
    return 0;
}
