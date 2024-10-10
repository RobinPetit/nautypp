#ifndef NAUTYPP_CLIQUER_HPP
#define NAUTYPP_CLIQUER_HPP

#include <vector>

/************/
#define new __new_var__  // this C file has functions with local variables called new...
// this is pretty hacky but C++ cannot implicit-cast void*
#define calloc(n, s) (set_t)calloc((n), (s))
extern "C" {
#include <nauty/nautycliquer.h>
}
#undef calloc
#undef new
/************/

#include <nautypp/aliases.hpp>

namespace nautypp {
class Graph;

typedef graph_t cliquer_graph_t;  // from nautycliquer.h

namespace Cliquer {
class Set {
public:
    class iterator {
    public:
        iterator() = delete;
        iterator(const iterator&) = delete;
        iterator(iterator&& other);
        iterator(const set_t s, Vertex v=0): set{s}, current{v} {
            _goto_next();
        }

        inline bool operator==(const iterator& other) const {
            return set == other.set and current == other.current;
        }
        inline bool operator!=(const iterator& other) const {
            return set != other.set or current != other.current;
        }

        inline Vertex operator*() const {
            return current;
        }
        inline iterator& operator++() {
            if(current < SET_MAX_SIZE(set))
                ++current;
            _goto_next();
            return *this;
        }
    private:
        const set_t set;
        Vertex current;

        inline void _goto_next() {
            while(current < SET_MAX_SIZE(set) and
                  not SET_CONTAINS(set, current))
                ++current;
        }
    };
    Set() = delete;
    Set(set_t set, bool copy=true): _vtx_set{nullptr}, host{false} {
        if(copy) {
            _vtx_set = set_duplicate(set);
            host = true;
        } else {
            _vtx_set = set;
        }
    }
    Set(Set&& other):
            _vtx_set{std::move(other._vtx_set)},
            host{other.host} {
        other.host = false;
    }

    ~Set() {
        if(host) {
            set_free(_vtx_set);
            _vtx_set = nullptr;
            host = false;
        }
    }

    inline iterator begin() const {
        return iterator(_vtx_set, 0);
    }
    inline iterator end() const {
        return iterator(_vtx_set, SET_MAX_SIZE(_vtx_set));
    }

    inline explicit operator std::vector<Vertex>() const {
        std::vector<Vertex> ret;
        ret.reserve(static_cast<size_t>(set_size(_vtx_set)));
        for(Vertex v : *this)
            ret.push_back(v);
        return ret;
    }
    inline size_t size() const {
        return static_cast<size_t>(set_size(_vtx_set));
    }
private:
    set_t _vtx_set;
    bool host;
};
}
}


#endif
