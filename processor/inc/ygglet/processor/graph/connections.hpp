#pragma once

#include <boost/iterator/iterator_facade.hpp>
#include <boost/uuid/uuid.hpp>

namespace ygglet::processor {
struct Connection;
} // namespace ygglet::processor

namespace ygglet::processor::graph {

template <typename G> struct Connections
{
    struct iterator : public boost::iterator_facade<iterator, Connection, boost::random_access_traversal_tag>
    {
        iterator() = default;

    private:
        friend class boost::iterator_core_access;
        friend struct Connections;
        using UnderlyingIterator = decltype(std::declval<G&>().m_graph.control.connections.begin());
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

    iterator begin() { return iterator(m_graph.m_graph.control.connections.begin()); }
    iterator end() { return iterator(m_graph.m_graph.control.connections.end()); }

    const_iterator begin() const { return begin(); }
    const_iterator end() const { return end(); }

private:
    friend G;

    Connections(G& graph)
    : m_graph(graph)
    {
    }

    G& m_graph;
};

} // namespace ygglet::processor::graph
