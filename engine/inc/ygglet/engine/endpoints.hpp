#pragma once

#include <boost/iterator/iterator_facade.hpp>
#include <boost/uuid/uuid.hpp>

namespace ygglet::engine {

template <typename E, typename D> struct Endpoints
{
    struct iterator : public boost::iterator_facade<iterator, boost::uuids::uuid, boost::forward_traversal_tag>
    {
        iterator() = default;

    private:
        friend class boost::iterator_core_access;
        friend struct Endpoints;
        using UnderlyingIterator = decltype(D::endpoints(std::declval<E&>()).begin());
        iterator(UnderlyingIterator endpoint)
        : m_endpoint(endpoint)
        {
        }
        boost::uuids::uuid dereference() const { *m_endpoint; }
        void increment() { ++m_endpoint; }
        bool equal(const iterator& other) const { return m_endpoint == other.m_endpoint; }
        UnderlyingIterator m_endpoint{};
    };

    iterator begin() { return iterator(D::endpoints(m_engine).begin()); }
    iterator end() { return iterator(D::endpoints(m_engine).end()); }

private:
    friend E;

    Endpoints(E& engine)
    : m_engine(engine)
    {
    }

    E& m_engine;
};

template <typename G> struct Inputs : Endpoints<G, Inputs<G>>
{
    static auto& endpoints(G& engine) { return engine.m_graph.control.inputs; }
};

template <typename G> struct Outputs : Endpoints<G, Outputs<G>>
{
    static auto& endpoints(G& engine) { return engine.m_graph.control.outputs; }
};

} // namespace ygglet::engine
