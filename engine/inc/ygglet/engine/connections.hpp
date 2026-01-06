#pragma once

#include <ygglet/engine/connection.hpp>

#include <boost/iterator/iterator_facade.hpp>
#include <boost/uuid/uuid.hpp>

namespace ygglet::engine {

template <typename E> struct Connections
{
    struct iterator : public boost::iterator_facade<iterator, Connection, boost::random_access_traversal_tag>
    {
        iterator() = default;

    private:
        friend class boost::iterator_core_access;
        friend struct Connections;
        using UnderlyingIterator = decltype(std::declval<E&>().m_graph.control.connections.begin());
        iterator(UnderlyingIterator itr)
        : m_itr(itr)
        {
        }
        Connection& dereference() const { return *m_itr; }
        void increment() { ++m_itr; }
        void decrement() { --m_itr; }
        void advance(std::ptrdiff_t n) { m_itr += n; }
        std::ptrdiff_t distance_to(const iterator& other) const { return other.m_itr - m_itr; }
        bool equal(const iterator& other) const { return m_itr == other.m_itr; }
        UnderlyingIterator m_itr;
    };

    using const_iterator = iterator;

    // TODO: transitions
    void insert(const Connection& connection)
        requires(!std::is_const_v<E>)
    {
        m_engine.m_graph.control.connections.insert(connection);
    }

    void remove(const Connection& connection)
        requires(!std::is_const_v<E>)
    {
        m_engine.m_graph.control.connections.erase(connection);
    }

    iterator begin() { return iterator(m_engine.m_graph.control.connections.begin()); }
    iterator end() { return iterator(m_engine.m_graph.control.connections.end()); }

    const_iterator begin() const { return begin(); }
    const_iterator end() const { return end(); }

private:
    friend E;

    Connections(E& engine)
    : m_engine(engine)
    {
    }

    E& m_engine;
};

} // namespace ygglet::engine
