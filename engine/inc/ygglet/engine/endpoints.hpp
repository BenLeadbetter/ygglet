#pragma once

#include <boost/iterator/iterator_facade.hpp>
#include <boost/uuid/generators.hpp>
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

    std::size_t size() const { return D::endpoints(m_engine).size(); }

    bool empty() const { return D::endpoints(m_engine).empty(); }

    void resize(std::size_t size)
        requires(!std::is_const_v<E>)
    {
        auto& e = D::endpoints(m_engine);
        auto old = e.size();
        e.resize(size);
        auto uuidgen = boost::uuids::random_generator{};
        for (auto i = old; i < size; ++i)
        {
            e[i] = uuidgen();
        }
    }

    boost::uuids::uuid operator[](std::size_t i) { return D::endpoints(m_engine)[i]; }

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
    static auto& endpoints(G& engine) { return engine.m_control.inputs; }
};

template <typename G> struct Outputs : Endpoints<G, Outputs<G>>
{
    static auto& endpoints(G& engine) { return engine.m_control.outputs; }
};

} // namespace ygglet::engine
