#include <atomic>
#include <iostream>
#include <vector>

#include <nautypp/nauty.hpp>

using namespace nautypp;

static constexpr int V{6};

static inline bool contains_triangle(const Graph& G) {
    std::vector<bool> marked(G.V(), false);
    std::vector<std::pair<Vertex, Vertex>> stack;
    stack.emplace_back(0, -1);
    while(not stack.empty()) {
        auto [v, parent] = stack.back();
        stack.pop_back();
        for(auto w : G.neighbours_of(v)) {
            if(marked[w]) {
                if(G.are_linked(parent, w))
                    return true;
                continue;
            } else {
                marked[w] = true;
                stack.emplace_back(w, v);
            }
        }
    }
    return false;
}

static unsigned through_all() {
    NautyParameters params{
        .tree=false,
        .connected=true,
        .triangle_free=false,
        .V=V,
        .Vmax=V
    };
    Nauty nauty(params);
    std::atomic_uint ret;
    nauty.run_async(
        [&ret](const Graph& G) {
            if(contains_triangle(G))
                return;
            ++ret;
        }
    );
    return ret;
}

static unsigned through_triangle_frees() {
    NautyParameters params{
        .tree=false,
        .connected=true,
        .triangle_free=true,
        .V=V,
        .Vmax=V
    };
    Nauty nauty(params);
    std::atomic_uint ret;
    nauty.run_async(
        [&ret](const Graph& /* G */) {
            ++ret;
        }
    );
    return ret;
}

int main() {
    unsigned first_count{through_all()};
    unsigned second_count{through_triangle_frees()};
    if(first_count != second_count) {
        std::cerr << "Problem: counted " << first_count << " and then " << second_count << std::endl;
    } else {
        std::cout<< "OK: " << first_count << " == " << second_count << std::endl;
    }
    return 0;
}
