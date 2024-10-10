#ifndef NAUTYPP_PROPERTIES_HPP
#define NAUTYPP_PROPERTIES_HPP

#include <utility>

#include <nautypp/aliases.hpp>
#include <nautypp/cliquer.hpp>

namespace nautypp {
class Graph;

template <typename T>
class ComputableProperty {
public:
    ComputableProperty() = delete;
    ComputableProperty(const Graph& G):
            graph{std::addressof(G)}, computed{false}, value{} {
    }
    ComputableProperty(const ComputableProperty&) = delete;
    ComputableProperty(ComputableProperty&& other):
            graph{other.graph}, computed{other.computed},
            value{std::move(other.value)} {
    }

    ComputableProperty& operator=(const ComputableProperty&) = delete;
    ComputableProperty& operator=(ComputableProperty&& other) {
        graph = other.graph;
        computed = other.computed;
        value = std::move(other.value);
        return *this;
    }

    virtual ~ComputableProperty() = default;
    inline void set_stale() {
        computed = false;
    }

    inline T get() {
        if(not computed) {
            compute();
            computed = true;
        }
        return value;
    }

    inline operator bool() const {
        return computed;
    }
protected:
    friend class Graph;

    const Graph* graph;
    bool computed=false;
    T value;
    virtual void compute() = 0;

    inline void reset_graph(const Graph* new_graph) {
        graph = new_graph;
    }
};

class EdgeProperty final : public ComputableProperty<size_t> {
public:
    EdgeProperty(const Graph& G): ComputableProperty(G) {
    }
    EdgeProperty(EdgeProperty&& other):
            ComputableProperty<size_t>(std::move(other)) {
    }
    EdgeProperty& operator=(EdgeProperty&& other) {
        ComputableProperty<size_t>::operator=(std::move(other));
        return *this;
    }
    virtual ~EdgeProperty() = default;
protected:
    virtual inline void compute() override final;
};

class DegreeProperty final : public ComputableProperty<size_t> {
public:
    DegreeProperty(const Graph& G, Vertex vertex):
            ComputableProperty<size_t>(G), v{vertex} {
    }
    DegreeProperty(DegreeProperty&& other):
            ComputableProperty<size_t>(std::move(other)), v{other.v} {
    }
    DegreeProperty(const DegreeProperty&) = delete;

    DegreeProperty& operator=(const DegreeProperty&) = delete;
    DegreeProperty& operator=(DegreeProperty&& other) {
        ComputableProperty<size_t>::operator=(std::move(other));
        v = other.v;
        return *this;
    }
    ~DegreeProperty() = default;
protected:
    Vertex v;

    virtual inline void compute() override final;
};

class _CliquerGraphProperty final : public ComputableProperty<cliquer_graph_t*> {
public:
    _CliquerGraphProperty(const Graph& G):
            ComputableProperty<cliquer_graph_t*>(G) {
    }
    _CliquerGraphProperty(_CliquerGraphProperty&& other):
            ComputableProperty<cliquer_graph_t*>(std::move(other)) {
    }
    _CliquerGraphProperty(const _CliquerGraphProperty&) = delete;

    _CliquerGraphProperty& operator=(const _CliquerGraphProperty&) = delete;
    inline _CliquerGraphProperty& operator=(_CliquerGraphProperty&& other) {
        ComputableProperty<cliquer_graph_t*>::operator=(std::move(other));
        return *this;
    }
    ~_CliquerGraphProperty() {
        clear();
    }
protected:
    virtual inline void compute() override final;

    inline void clear() {
        if(value != nullptr) {
            graph_free(value);
            value = nullptr;
        }
    }
};

}

#endif
