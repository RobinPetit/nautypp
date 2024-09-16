#ifndef __NAUTY_WRAPPER_HPP__
#define __NAUTY_WRAPPER_HPP__

#include <algorithm>
#include <bit>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <list>
#include <mutex>
#include <thread>
#include <vector>

#ifdef __linux__
#include <pthread.h>
#endif

#include "nauty/gtools.h"

#ifdef NAUTYPP_SGO
# ifndef NAUTYPP_SMALL_GRAPH_SIZE
// graphs with less than that many vertices do not need to be dynamically
// allocated (similar to SSO)
#  define NAUTYPP_SMALL_GRAPH_SIZE 0
# endif
#endif

extern int _geng_main(int, char**);
extern int _gentreeg_main(int, char**);

static inline void rename_thread(std::thread& thread, const char* name) {
#ifdef __linux__
    pthread_setname_np(
        thread.native_handle(),
        name
    );
#else
    (void)(thread);
    (void)(name);
#endif
}

namespace nautypp {

typedef setword xword;  // from geng.c
typedef graph nauty_graph_t;
typedef xword Vertex;

class Graph;

struct NautyParameters {
    bool tree{false};
    bool connected{true};
    bool triangle_free{false};
    int V{-1};
    int Vmax{-1};
    int min_deg{-1};
    int max_deg{std::numeric_limits<int>::max()};
};

struct NautyThreadLauncher {
    static void start_gentreeg(int argc, char** argv);
};


/* ******************** Concepts ******************** */

template <typename T>
concept IsVoid = std::is_same_v<T, void>;

template <typename T>
struct IsGraphFunctionConstRef :
        public std::integral_constant<
            bool,
            std::is_invocable_v<T, const Graph&>
            && IsVoid<std::invoke_result_t<T, const Graph&>>
        > {
};

template <typename T>
struct IsGraphFunctionRef :
        public std::integral_constant<
            bool,
            std::is_invocable_v<T, Graph&>
            && IsVoid<std::invoke_result_t<T, Graph&>>
        > {
};

template <typename T>
concept GraphRefFunctionType = requires(T obj, Graph& G) {
    { obj(G) };
};
template <typename T>
concept GraphConstRefFunctionType = requires(T obj, const Graph& G) {
    { obj(G) };
};
template <typename T>
concept GraphFunctionType = GraphRefFunctionType<T>
                         or GraphConstRefFunctionType<T>;
template <typename T>
concept GraphCallbackType = requires(T obj, Graph& G) {
    { obj(G) };
};

/* ******************** Types ******************** */

/*      *************** Iterables and Iterators ***************      */

class EdgeIterator {
public:
    EdgeIterator() = delete;
    EdgeIterator(const Graph&, Vertex vertex, bool end=false);
    inline bool operator==(const EdgeIterator&) const;
    inline bool operator!=(const EdgeIterator&) const;
    inline EdgeIterator& operator=(EdgeIterator&& other);
    inline EdgeIterator& operator++();
    inline std::pair<Vertex, Vertex> operator*() const;
private:
    const Graph& graph;
    xword v, w;
    setword gv;
};

class AllEdgeIterator {
public:
    AllEdgeIterator() = delete;
    AllEdgeIterator(const Graph&, bool end=false);
    inline bool operator==(const AllEdgeIterator&) const;
    inline bool operator!=(const AllEdgeIterator&) const;
    inline AllEdgeIterator& operator++();
    inline std::pair<Vertex, Vertex> operator*() const;
private:
    const Graph& graph;
    EdgeIterator it;

    inline void next();
};

class Edges {
public:
    Edges() = delete;
    Edges(const Graph& G, xword vertex);

    inline EdgeIterator begin() const {
        return {graph, v};
    }
    inline EdgeIterator end() const {
        return {graph, v, true};
    }
private:
    const Graph& graph;
    xword v;
};

class AllEdges {
public:
    AllEdges() = delete;
    AllEdges(const Graph& G);

    inline AllEdgeIterator begin() const {
        return {graph, false};
    }
    inline AllEdgeIterator end() const {
        return {graph, true};
    }

    operator std::vector<std::pair<Vertex, Vertex>>() const;

    inline std::vector<std::pair<Vertex, Vertex>> as_vector() const {
        return *this;
    }
private:
    const Graph& graph;
};

/*      *************** Graph Properties ***************      */

template <typename T>
class ComputableProperty {
public:
    ComputableProperty() = delete;
    ComputableProperty(const Graph& G):
            graph{std::addressof(G)}, computed{false}, value() {
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
protected:
    Vertex v;

    virtual inline void compute() override final;
};

/*      *************** Graph ***************      */

class BaseGraph {
public:
    BaseGraph() = default;
    virtual ~BaseGraph() = default;

    virtual inline size_t V() const = 0;
    virtual inline size_t E() const = 0;

    virtual inline bool are_linked(Vertex v, Vertex w) const = 0;
    virtual inline size_t degree(Vertex v) const = 0;
    virtual inline std::vector<size_t> degree() const = 0;
    virtual inline size_t delta() const = 0;
    inline size_t min_degree() const { return delta(); }
    virtual inline size_t Delta() const = 0;
    inline size_t max_degree() const { return Delta(); }
    virtual inline std::pair<size_t, size_t> delta_Delta() const = 0;
    virtual inline std::pair<size_t, size_t> minmax_degree() const { return delta_Delta(); }

    virtual inline std::vector<Vertex> neighbours_of(Vertex v) const = 0;
};

class Graph final : virtual public BaseGraph {
public:
    Graph() = delete;
    Graph(const graph* G, size_t V);
    Graph(int* parents, size_t);  // tree construction
    Graph(graph* G, size_t V, bool copy=true);
    Graph(size_t V);
    Graph(Graph&& G);
    Graph(const Graph&) = delete;

    inline Graph copy() const {
        return Graph(g, n, true);
    }

    Graph& operator=(const Graph& other) = delete;
    Graph& operator=(Graph&& other);

    ~Graph() {
        reset();
    }

    virtual inline size_t V() const override final {
        return n;
    }

    virtual inline size_t E() const override final {
        return non_const_self().nb_edges.get();
    }

    inline Edges edges(Vertex v) const {
        return Edges(*this, v);
    }

    inline AllEdges edges() const {
        return AllEdges(*this);
    }

    inline explicit operator const graph*() const {
        return g;
    }

    virtual inline size_t degree(Vertex v) const {
        if(degrees.size() < V())
            non_const_self().init_degrees();
#ifdef NAUTYPP_DEBUG
        return non_const_self().degrees.at(v).get();
#else
        return non_const_self().degrees[v].get();
#endif
    }

    virtual inline std::vector<size_t> degree() const final {
        std::vector<size_t> ret(V());
        for(Vertex v{0}; v < V(); ++v)
            ret[v] = degree(v);
        return ret;
    }

    virtual inline size_t delta() const final {
        auto degrees{degree()};
        return *std::min_element(degrees.cbegin(), degrees.cend());
    }

    virtual inline size_t Delta() const final {
        auto degrees{degree()};
        return *std::max_element(degrees.cbegin(), degrees.cend());
    }

    virtual inline std::pair<size_t, size_t> delta_Delta() const {
        auto degrees{degree()};
        auto [min_it, max_it] = std::minmax_element(degrees.cbegin(), degrees.cend());
        return {*min_it, *max_it};
    }

    typedef std::vector<std::pair<size_t, size_t>> DegreeDistribution;

    inline DegreeDistribution degree_distribution() const {
        std::vector<size_t> counts(V(), 0);
        for(Vertex v{0}; v < V(); ++v)
            ++counts[degree(v)];
        std::vector<std::pair<size_t, size_t>> ret;
        for(size_t d{0}; d < V(); ++d) {
            auto count{counts[d]};
            if(count == 0)
                continue;
            ret.emplace_back(d, count);
        }
        return ret;
    }

    virtual inline bool are_linked(Vertex v, Vertex w) const final {
        return ISELEMENT(
            GRAPHROW(g, v, _m),
            w
        );
    }

    inline void link(Vertex v, Vertex w) {
        ADDELEMENT(GRAPHROW(g, v, _m), w);
        ADDELEMENT(GRAPHROW(g, w, _m), v);
        degrees[v].set_stale();
        degrees[w].set_stale();
        nb_edges.set_stale();
    }

    virtual inline std::vector<Vertex> neighbours_of(Vertex v) const final {
        std::vector<Vertex> ret;
        auto degv{degree(v)};
        if(degv == 0)
            return ret;
        ret.reserve(degv);
        for(auto [_, w] : edges(v))
            ret.push_back(w);
        return ret;
    }

    inline Vertex some_neighbour_of(Vertex v) const {
        auto [_, w] = *edges(v).begin();
        return w;
    }

    /* NO VERIFICATION OF DEGREE IS MADE HERE! */
    inline Vertex some_neighbour_of_other_than(Vertex v, Vertex w) const {
        auto it{edges(v).begin()};
        if((*it).second != w)
            return (*it).second;
        else
            return (*++it).second;
    }

    inline bool is_leaf(Vertex v) const {
        return degree(v) == 1;
    }

    inline size_t leaf_degree(Vertex v) const {
        size_t ret{0};
        for(auto w : neighbours_of(v))
            if(is_leaf(w))
                ++ret;
        return ret;
    }

    inline std::vector<size_t> leaf_degree() const {
        std::vector<size_t> ret(V(), 0);
        for(Vertex v{0}; v < V(); ++v)
            if(degree(v) == 1)
                ++ret[first_neighbour_of_nz(v)];
        return ret;
    }

    inline size_t max_leaf_degree() const {
        auto leaf_neighbours{leaf_degree()};
        return *std::max_element(
            leaf_neighbours.begin(), leaf_neighbours.end()
        );
    }

    inline bool max_leaf_degree_bounded_by(size_t d) const {
        std::vector<size_t> leaf_neighbours(V(), 0);
        for(Vertex v{0}; v < V(); ++v)
            if(degree(v) == 1)
                if(++leaf_neighbours[first_neighbour_of_nz(v)] > d)
                    return false;
        return true;
    }

    inline void isolate_vertex(Vertex v) {
        setword i;
        setword* Nv{setwordof(v)};
        while((i = FIRSTBIT(*Nv)) != WORDSIZE) {
            DELELEMENT(setwordof(i), static_cast<setword>(v));
            DELELEMENT(Nv, i);
            degrees[i].set_stale();
        }
        degrees[v].set_stale();
        nb_edges.set_stale();
    }

    friend class EdgeIterator;
    friend class NautyContainer;
    friend class EdgeProperty;
    friend class DegreeProperty;

/* ******************** Static Methods ******************** */

/*      *************** Creation of Graph From Other Representations ***************      */
    /* NO LOOPS! */
    template <typename T>
    inline static Graph from_adjacency_matrix(
            const std::vector<T> A, bool upper=true) {
        auto V{static_cast<size_t>(
            std::sqrt(static_cast<float>(A.size())) + .5
        )};
        return from_adjacency_matrix(A, V, upper);
    }

    /* NO LOOPS! */
    template <typename T>
    inline static Graph from_adjacency_matrix(
            const std::vector<T> A,
            size_t V, bool upper=true) {
        if(upper)
            return from_adjacency_matrix_upper(A, V);
        else
            return from_adjacency_matrix_lower(A, V);
    }

    template <typename T>
    static Graph from_adjacency_matrix_upper(const std::vector<T> A, size_t V) {
        if(V*V != A.size())
            throw std::runtime_error("Wrong dimensions for adjacency matrix");
        Graph ret(V);
        for(Vertex v{0}; v+1 < V; ++v)
            for(Vertex w{v+1}; w < V; ++V)
                if(A.at(v*V + w))
                    ret.link(v, w);
        return ret;
    }

    template <typename T>
    static Graph from_adjacency_matrix_lower(const std::vector<T> A, size_t V) {
        if(V*V != A.size())
            throw std::runtime_error("Wrong dimensions for adjacency matrix");
        Graph ret(V);
        for(Vertex v{1}; v < V; ++v)
            for(Vertex w{0}; w < v; ++V)
                if(A.at(v*V + w))
                    ret.link(v, w);
        return ret;
    }

/*      *************** Construction of Graph Families ***************      */

    /* return P_N */
    inline static Graph make_path(size_t N) {
        Graph ret(N);
        for(Vertex v{1}; v < N; ++v)
            ret.link(v-1, v);
        return ret;
    }

    /* return C_N */
    inline static Graph make_cycle(size_t N) {
        auto ret{make_path(N)};
        ret.link(0, N-1);
        return ret;
    }

    /* return K_{1,N} */
    inline static Graph make_claw(size_t N) {
        Graph ret(N+1);
        for(Vertex v{0}; v < N; ++v)
            ret.link(v, N);
        return ret;
    }

    /* alias for make_claw */
    inline static Graph make_star(size_t N) {
        return make_claw(N);
    }

    /* return K_N */
    inline static Graph make_complete(size_t N) {
        Graph ret(N);
        for(Vertex v{0}; v < N; ++v)
            for(Vertex w{v+1}; w < N; ++w)
                ret.link(v, w);
        return ret;
    }

    /* return K_{s,t} */
    inline static Graph make_complete_bipartite(size_t s, size_t t) {
        Graph ret(s+t);
        for(Vertex v{0}; v < s; ++v)
            for(Vertex w{s}; w < s+t; ++w)
                ret.link(v, w);
        return ret;
    }
private:
    size_t n;
    bool host;
#ifdef NAUTYPP_SGO
    graph __small_graph_buffer[NAUTYPP_SMALL_GRAPH_SIZE];
#endif
    graph* g;

    static inline constexpr size_t _m{1};

    EdgeProperty nb_edges;
    std::vector<DegreeProperty> degrees;

    inline void reset() {
        if(not host or g == nullptr)
            return;
        delete[] g;
        g = nullptr;
        host = false;
    }

    inline void assign_from(const graph* G, size_t V) {
#ifdef NAUTYPP_SGO
        if(V <= NAUTYPP_SMALL_GRAPH_SIZE) {
            g = __small_graph_buffer;
            host = false;
        } else {
            g = new graph[_m*V];
            host = true;
        };
#else
        g = new graph[_m*V];
        host = true;
#endif
        memcpy(g, G, _m*V*sizeof(*G));
    }

    inline void init_degrees() {
        degrees.reserve(n);
        for(size_t v{degrees.size()}; v < V(); ++v)
            degrees.emplace_back(*this, v);
    }

    inline size_t __get_m() const {
        return _m;
    }

    inline Graph& non_const_self() const {
        return *const_cast<Graph*>(this);
    }

    inline Vertex first_neighbour_of(Vertex v) const {
        setword Nv{*setwordof(v)};
        Vertex ret{static_cast<Vertex>(FIRSTBIT(Nv))};
        if(ret == WORDSIZE)
            throw std::runtime_error("Vertex has no neighbour");
        return ret;
    }

    inline Vertex first_neighbour_of_nz(Vertex v) const {
        return static_cast<Vertex>(FIRSTBITNZ(*GRAPHROW(g, v, _m)));
    }

    static inline size_t m_from_n(size_t n) {
        return 1 + (n-1) / WORDSIZE;
    }

    inline setword* setwordof(Vertex v) {
        return GRAPHROW(g, v, _m);
    }

    inline const setword* setwordof(Vertex v) const {
        return GRAPHROW(g, v, _m);
    }
};

bool EdgeIterator::operator==(const EdgeIterator& other) const {
    return std::addressof(graph) == std::addressof(other.graph)
       and v == other.v
       and gv == other.gv
       and w == other.w;
}

bool EdgeIterator::operator!=(const EdgeIterator& other) const {
    return not (*this == other);
}

EdgeIterator& EdgeIterator::operator=(EdgeIterator&& other) {
    if(std::addressof(graph) != std::addressof(other.graph))
        throw std::runtime_error(
            "Cannot reassign EdgeIterator to a new graph"
        );
    v = other.v;
    w = other.w;
    gv = other.gv;
    return *this;
}

EdgeIterator& EdgeIterator::operator++() {
    if(gv > 0) {
        TAKEBIT(w, gv);
    } else {
        w = WORDSIZE;
    }
    return *this;
}

std::pair<Vertex, Vertex> EdgeIterator::operator*() const {
    return {v, w};
}

bool AllEdgeIterator::operator==(const AllEdgeIterator& other) const {
    return it == other.it;
}

bool AllEdgeIterator::operator!=(const AllEdgeIterator& other) const {
    return not (*this == other);
}

AllEdgeIterator& AllEdgeIterator::operator++() {
    do {
        next();
    } while((*it).first > (*it).second);
    return *this;
}

void AllEdgeIterator::next() {
    ++it;
    auto [v, w] = *it;
    if(w == WORDSIZE and v+1 < graph.V())
        it = EdgeIterator(graph, v+1, false);
}

std::pair<Vertex, Vertex> AllEdgeIterator::operator*() const {
    return *it;
}

void DegreeProperty::compute() {
    value = std::popcount(*GRAPHROW(
        static_cast<const nauty_graph_t*>(*graph),
        v,
        graph->__get_m()
    ));
}

void EdgeProperty::compute() {
    value = 0;
    for(size_t v{0}; v < graph->V(); ++v)
        value += graph->degree(v);
    value >>= 1;  // sum(d(v)) == 2E
}

/* ******************** Multithreading Tools ******************** */

enum class NautyStatus {
    DATA_AVAILABLE,
    END_OF_THREAD
};

// forward declarations
class Nauty;
template <GraphFunctionType Callback>
class NautyWorker;

/*      *************** Containers ***************      */

template <typename T>
class ContainerBuffer {
public:
    ContainerBuffer() = delete;
    ContainerBuffer(size_t maxsize):
            _size{maxsize},
            _read_buffer(), _write_buffer(),
            _writable{true}, _should_swap{false} {
        _read_buffer.reserve(maxsize);
        _write_buffer.reserve(maxsize);
    }

    ContainerBuffer(const ContainerBuffer&) = delete;
    ContainerBuffer(ContainerBuffer&&) noexcept = default;

    ~ContainerBuffer() {
        if(_read_buffer.size() > 0)
            std::cerr << "Destroying a buffer w/ reading capability";
        if(_write_buffer.size() > 0)
            std::cerr << "Missed a swap (" << _write_buffer.size() << ")\n";
    }

    inline bool push(T& G) {
        if(_should_swap or write_size() == _size) {
            _should_swap = true;
            return false;
        }
        _write_buffer.push_back(std::move(G));
        return true;
    }

    class EmptyBuffer : public std::exception {
    };

    inline T pop() {
        do {
            __verify_not_empty_read();
        } while(_read_buffer.empty() and _writable);
        if(_read_buffer.empty()) {
            if(write_size() > 0)
                std::cerr << "SHOULD NOT THROW\n";
            throw EmptyBuffer();
        }
        T ret{std::move(_read_buffer.back())};
        _read_buffer.pop_back();
        return ret;
    }

    inline bool writable() const {
        return _writable;
    }

    inline void enable_write() {
        _writable = true;
    }

    inline void disable_write() {
        _should_swap = true;
        _writable = false;
    }
private:
    size_t                    _size;
    std::vector<T>            _read_buffer;
    std::vector<T>            _write_buffer;
    volatile std::atomic_bool _writable;
    volatile std::atomic_bool _should_swap;

    inline void __verify_not_empty_read() {
        using namespace std::chrono_literals;
        if(read_size() > 0) {
            return;
        } else {
            while(_writable and not _should_swap)
                std::this_thread::sleep_for(10ms);
            if(_should_swap)
                std::swap(_read_buffer, _write_buffer);
            _should_swap = false;
        }
    }

    /* NO LOCK IS PERFORMED HERE! */
    inline size_t read_size() const {
        return _read_buffer.size();
    }

    /* NO LOCK IS PERFORMED HERE! */
    inline size_t write_size() const {
        return _write_buffer.size();
    }
};

typedef ContainerBuffer<Graph> NautyContainerBuffer;

typedef std::function<bool(const Graph&)> GraphPredicate;

template <typename... Args>
bool __true(const Args&... args) {
    return true;
}

#ifndef OSTREAM_LL_GRAPH
#define OSTREAM_LL_GRAPH
static std::ostream& operator<<(std::ostream& os, const Graph& G) {
    os << "Graph(";
    bool first{true};
    for(auto [v, w] : G.edges()) {
        if(first)
            first = false;
        else
            os << ", ";
        os << v << "-" << w;
    }
    return os << ")";
}
#endif

class NautyContainer {
public:
    NautyContainer():
            _done{false}, _worker_buffers(), _buffer_idx{0},
            _predicate{__true<Graph>} {
    }

    inline std::shared_ptr<NautyContainerBuffer> add_new_buffer(size_t buffer_size) {
        _worker_buffers.push_back(
            std::move(std::make_shared<NautyContainerBuffer>(buffer_size))
        );
        return _worker_buffers.back();
    }

    template <typename... Args>
    inline void emplace(Args&&... args) {
        Graph G(std::forward<Args>(args)...);
        if(not _predicate(G)) {
            return;
        }
        while(true) {
            for(auto& buffer : _worker_buffers) {
                if(buffer->writable() and buffer->push(G))
                    return;
            }
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(1ms);
        }
    }

    inline void _add_gentree_tree(int* parents, size_t n) {
        emplace(parents, n);
    }

    friend class NautyThreadLauncher;
    friend class Nauty;
private:
    typedef std::vector<std::shared_ptr<NautyContainerBuffer>> ContainerVector;
    bool            _done;
    ContainerVector _worker_buffers;
    size_t          _buffer_idx;
    GraphPredicate  _predicate;

    inline void set_over() {
        for(auto& buffer : _worker_buffers)
            buffer->disable_write();
    }

    inline void set_predicate(GraphPredicate p) {
        _predicate = p;
    }
};

/*      *************** Workers ***************      */

class BaseNautyWorker {
public:
    BaseNautyWorker(std::shared_ptr<NautyContainerBuffer> buffer):
            _buffer{std::move(buffer)} {
    }

    ~BaseNautyWorker() = default;

    void run() {
        while(true) {
            try {
                Graph G{std::move(_buffer->pop())};
                (*this)(G);
            } catch(NautyContainerBuffer::EmptyBuffer&) {
                break;
            }
        }
    }

    virtual void operator()(Graph& G) = 0;
protected:
    typedef std::shared_ptr<NautyContainerBuffer> BufferPtr;
    BufferPtr  _buffer;
};

template <GraphFunctionType GraphFunction>
class NautyWorker final : public BaseNautyWorker {
public:
    typedef GraphFunction callback_t;

    NautyWorker(std::shared_ptr<NautyContainerBuffer> buffer, callback_t callback):
            BaseNautyWorker(buffer), _callback{callback} {
    }

    NautyWorker(NautyWorker&) = delete;
    NautyWorker(NautyWorker&&) = default;

#ifdef NAUTYPP_DEBUG
    ~NautyWorker() noexcept(false) {
        if(_buffer->_writable)
            throw std::runtime_error("Destroying a worker with a readable buffer.");
        if(_buffer->read_size() > 0)
            std::cerr << "Destroying worker with read buffer size " << _buffer->read_size() << std::endl;
    }
#else
    ~NautyWorker() = default;
#endif

    virtual void operator()(Graph& G) override final {
        _callback(G);
    }
protected:
    callback_t _callback;
};

template <GraphCallbackType Callback>
class NautyWorkerWrapper final : public BaseNautyWorker {
public:
    NautyWorkerWrapper(std::shared_ptr<NautyContainerBuffer> buffer):
            BaseNautyWorker(buffer), _callback() {
    }

    virtual void operator()(Graph& G) override final {
        _callback(G);
    }

    inline void join(NautyWorkerWrapper& other) {
        _callback.join(other._callback);
    }

    inline const typename Callback::ResultType& get() const {
        return _callback.get();
    }
private:
    Callback _callback;
};

/*      *************** Nauty ***************      */

class Nauty {
public:
    Nauty(const NautyParameters& params):
            parameters{params} {
        if(parameters.connected)
            parameters.min_deg = std::max(1, parameters.min_deg);
        parameters.max_deg = std::min(parameters.Vmax-1, parameters.max_deg);
        if(parameters.Vmax < parameters.V)
            parameters.Vmax = parameters.V;
    }

    template <GraphFunctionType GraphFunction>
    void run_async(GraphFunction callback,
            size_t nb_workers=std::thread::hardware_concurrency(),
            size_t worker_buffer_size=5'000,
            GraphPredicate p=__true<Graph>) {
        Nauty::reset_container();
        Nauty::_container->set_predicate(p);
        std::vector<std::thread> worker_threads;
        std::vector<NautyWorker<GraphFunction>> workers;
        worker_threads.reserve(nb_workers);
        workers.reserve(nb_workers);
        static char name_buffer[32];
        for(size_t i{0}; i < nb_workers; ++i) {
            auto worker_buffer{Nauty::_container->add_new_buffer(worker_buffer_size)};
            workers.emplace_back(worker_buffer, callback);
            worker_threads.emplace_back(
                &NautyWorker<GraphFunction>::run,
                &workers.back()
            );
            std::sprintf(name_buffer, "Worker %u", static_cast<unsigned>(i+1));
            rename_thread(worker_threads.back(), name_buffer);
        }
        auto geng_thread{start_nauty()};
        geng_thread.join();
        for(size_t i{0}; i < nb_workers; ++i)
            worker_threads.at(i).join();
    }

    template <GraphCallbackType Callback>
    auto run_async(
            size_t nb_workers=std::thread::hardware_concurrency(),
            size_t worker_buffer_size=5'000,
            GraphPredicate p=__true<Graph>) -> Callback::ResultType {
        Nauty::reset_container();
        Nauty::_container->set_predicate(p);
        std::vector<NautyWorkerWrapper<Callback>> wrappers;
        std::vector<std::thread> workers;
        wrappers.reserve(nb_workers);
        workers.reserve(nb_workers);
        static char name_buffer[32];
        for(size_t i{0}; i < nb_workers; ++i) {
            auto worker_buffer{Nauty::_container->add_new_buffer(worker_buffer_size)};
            wrappers.emplace_back(worker_buffer);
            workers.emplace_back(
                &NautyWorkerWrapper<Callback>::run,
                std::addressof(wrappers.back())
            );
            std::sprintf(name_buffer, "Worker %u", static_cast<unsigned>(i+1));
            rename_thread(workers.back(), name_buffer);
        }
        auto geng_thread{start_nauty()};
        geng_thread.join();
        for(size_t i{0}; i < nb_workers; ++i)
            workers.at(i).join();
        for(size_t idx{1}; idx < nb_workers; ++idx)
            wrappers.at(0).join(wrappers.at(idx));
        return wrappers.at(0).get();
    }

    static inline std::unique_ptr<NautyContainer>& get_container() {
#ifdef NAUTYPP_DEBUG
        if(not Nauty::_container)
            throw std::runtime_error("No initialized container");
#endif
        return Nauty::_container;
    }
private:
    NautyParameters parameters;

    inline std::thread start_nauty() {
        std::thread ret{std::move(
            parameters.tree
            ? start_gentreeg()
            : start_geng()
        )};
#ifdef __linux__
        rename_thread(
            ret,
            parameters.tree
            ? "nauty-gentreeg"
            : "nauty-geng"
        );
#endif
        return ret;
    }

    inline std::thread start_gentreeg() {
        strcpy(params[0], "gentreeg");
        std::sprintf(
            params[1],
            "-D%d%s",
            parameters.max_deg,
#ifdef NAUTYPP_DEBUG
            ""
#else
            "q"
#endif
        );
        std::sprintf(
            params[2], "%d", parameters.V
        );
        for(auto i{0}; i < Nauty::GENG_ARGC; ++i)
            geng_argv[i] = params[i];
        std::thread t(
                NautyThreadLauncher::start_gentreeg,
                Nauty::GENG_ARGC,
                geng_argv
        );
        return t;
    }

    inline std::thread start_geng() {
        strcpy(params[0], "geng");
        for(auto i{0}; i < Nauty::GENG_ARGC; ++i)
            geng_argv[i] = params[i];
        std::thread t(
            [this](std::unique_ptr<NautyContainer>& container) {
                int min_deg, max_deg;
                for(int V{this->parameters.V}; V <= this->parameters.Vmax; ++V) {
                    auto& parameters{this->parameters};
                    min_deg = std::min(parameters.min_deg, V-1);
                    max_deg = std::min(parameters.max_deg, V-1);
                    std::sprintf(
                        params[1],
                        "-%sd%dD%d%s%s",
                        parameters.connected ? "c" : "",
                        min_deg,
                        max_deg,
                        parameters.triangle_free ? "t" : "",
#ifdef NAUTYPP_DEBUG
                        ""
#else
                        "q"
#endif
                    );
                    std::sprintf(this->geng_argv[2], "%d", V);
                    _geng_main(Nauty::GENG_ARGC, this->geng_argv);
                }
                container->set_over();
            },
            std::ref(get_container())
        );
        return t;
    }

    static inline constexpr auto GENG_ARGV_BUFFER_SIZE{32};
    static inline constexpr auto GENG_ARGC{3};
    char params[Nauty::GENG_ARGC][Nauty::GENG_ARGV_BUFFER_SIZE];
    char* geng_argv[Nauty::GENG_ARGC];

    static void reset_container() {
        Nauty::_container.reset(new NautyContainer());
    }

    static std::unique_ptr<NautyContainer> _container;
};

}

#endif
