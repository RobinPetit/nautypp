#ifndef NAUTYPP_ALGORITHMS_HPP
#define NAUTYPP_ALGORITHMS_HPP

#include <limits>
#include <stdexcept>
#include <vector>

#include <nautypp/aliases.hpp>

namespace nautypp {
class Graph;

class ConnectedComponents {
public:
    ConnectedComponents() = delete;
    inline ConnectedComponents(const Graph& graph);

    inline size_t get_nb_components() const {
        return nb_components;
    }
    inline size_t get_component_identifier_of(Vertex v) const {
        _verify_exists(v);
        return ids[v];
    }
    inline std::vector<Vertex> get_component_of(Vertex v) const {
        std::vector<Vertex> ret;
        ret.reserve(ids.size());
        for(Vertex w{0}; w < ids.size(); ++w)
            if(ids[w] == ids[v])
                ret.push_back(w);
        return ret;
    }
private:
    static constexpr size_t UNVISITED{std::numeric_limits<size_t>::max()};
    const Graph& G;
    std::vector<size_t> ids;
    size_t nb_components;

    inline void _run();
    void _run(Vertex v);
    inline void _verify_exists(Vertex v) const {
        if(v >= ids.size())
            throw std::runtime_error("Vertex does not exist");
    }
};
}

#endif
