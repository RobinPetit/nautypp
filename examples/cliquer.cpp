#include <iostream>
#include <vector>

#define OSTREAM_LL_GRAPH
#define OSTREAM_LL_CLIQUE
#include <nautypp/nautypp>

using namespace nautypp;

template <typename T>
static std::ostream& operator<<(std::ostream& os, const std::vector<T>& v) {
    os << '{';
    bool first{true};
    for(auto& x : v) {
        if(first)
            first = false;
        else
            os << ", ";
        os << x;
    }
    return os << "}";
}

static inline bool print_clique(const std::vector<Vertex>& clique) {
    std::cout << clique << std::endl;
    return true;
}

static inline boolean _from_cliquer(set_t clique, cliquer_graph_t*, clique_options*) {
    set_print(clique);
    return true;
}

static inline clique_options null_options() {
    return {
        .reorder_function=NULL,
        .reorder_map=NULL,
        .time_function=NULL,
        .output=NULL,
        .user_function=NULL,
        .user_data=NULL,
        .clique_list=NULL,
        .clique_list_length=0
    };
}

int main() {
    auto K5{Graph::make_complete(5)};
    std::cout << "Maximal clique in K5: ";
    std::cout << K5.find_some_clique(0, 0, true);
    std::cout << std::endl;

    std::cout << "\nList of all the (non-necessarily maximal) cliques in K5 of size >= 3:\n";
    for(auto& clique : K5.get_all_cliques(3, K5.V(), false)) {
        std::cout << clique << std::endl;
    }

    std::cout << "\nRetrieve all the cliques with a callback:\n";
    std::vector<std::vector<Vertex>> all_cliques;
    K5.apply_to_cliques(
        1, K5.V(), false,
        [&all_cliques](const std::vector<Vertex>& clique) -> bool {
            all_cliques.push_back(clique);
            return true;
        }
    );
    for(auto& clique : all_cliques)
        std::cout << clique << std::endl;

    std::cout << "\nCall `print_clique` on every generated clique of size >= 3:\n";
    K5.apply_to_cliques(3, K5.V(), false, print_clique);

    {
    std::cout << "\nCall `_from_cliquer` on every generated clique of size >= 3 with options:\n";
    clique_options opts{null_options()};
    opts.user_function = _from_cliquer;
    K5.apply_to_cliques(3, K5.V(), false, &opts);
    }

    {
    clique_options opts{null_options()};
    opts.clique_list_length = 10;
    opts.clique_list = new set_t[10];
    // or
    // opts.clique_list = (set_t*)malloc(10*sizeof(set_t));
    std::cout << "\nUsing the original cliquer interface. Only 10 cliques are stored (by choice):\n";
    K5.apply_to_cliques(3, K5.V(), false, &opts);
    for(int i{0}; i < opts.clique_list_length; ++i) {
        set_print(opts.clique_list[i]);
        set_free(opts.clique_list[i]);
    }
    delete[] opts.clique_list;
    // or
    // free(opts.clique_list);
    }
    return 0;
}
