#include <nautypp/nautypp>

using namespace nautypp;

static void test_planarity_of_Kn(size_t n) {
    auto Kn{Graph::make_complete(n)};
    bool isplanar{Kn.is_planar()};
    std::cout << "K_" << n << " is "
              << (isplanar ? "" : "not ") << "planar.\n";
}

static void test_planarity_of_Knn(size_t n) {
    auto Knn{Graph::make_complete_bipartite(n, n)};
    bool isplanar{Knn.is_planar()};
    std::cout << "K_" << n << "," << n << " is "
              << (isplanar ? "" : "not ") << "planar.\n";
}

int main() {
    for(size_t n{2}; n < 10; ++n)
        test_planarity_of_Kn(n);
    std::cout << std::endl;
    for(size_t n{2}; n < 10; ++n)
        test_planarity_of_Knn(n);
    return 0;
}
