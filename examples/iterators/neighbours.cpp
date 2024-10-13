#include <iostream>

#include <nautypp/nautypp>

using namespace nautypp;

int main() {
    Graph K5{Graph::make_complete(5)};
    Edges neighbours_of_2{K5.edges(2)};
    std::cout << "Edges incident to 2:" << std::endl;
    for(auto [v, w] : neighbours_of_2)
        std::cout << v << "-" << w << std::endl;
    std::cout << std::endl;
    std::cout << "All edges in K_5:" << std::endl;
    for(auto [v, w] : K5.edges())
        std::cout << v << "-" << w << std::endl;
    std::cout << std::endl;
    std::cout << "Neighbours of 3:" << std::endl;
    for(auto v : K5.neighbours_of(3))
        std::cout << v << "  ";
    std::cout << std::endl;
    std::cout << "Just one neighbour of 1: ";
    Vertex w{K5.some_neighbour_of(1)};
    std::cout << w << std::endl;
    Vertex u{K5.some_neighbour_of_other_than(1, w)};
    std::cout << "Another neighbour: " << u << std::endl;
    return 0;
}
