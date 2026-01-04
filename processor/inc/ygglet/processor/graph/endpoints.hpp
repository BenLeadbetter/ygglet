#pragma once

#include <boost/iterator/iterator_facade.hpp>
#include <boost/uuid/uuid.hpp>

namespace ygglet::processor::graph {

template <typename G, typename T> struct Endpoints
{
    struct Iterator : public boost::iterator_facade<Iterator, boost::uuids::uuid, boost::forward_traversal_tag>
    {
        Iterator() = default;

    private:
        friend class boost::iterator_core_access;
        friend struct Endpoints;
        using NodeIterator = decltype(std::declval<G&>().m_graph.control.nodes.begin());
        Iterator(const G& graph, NodeIterator node)
        : m_graph(graph)
        , m_node(node)
        {
        }
        boost::uuids::uuid dereference() const { return T::endpoints(*m_node->second)[m_index]; }
        void increment()
        {
            ++m_index;
            if (m_index == T::endpoints(*m_node->second).size())
            {
                ++m_node;
                m_index = 0;
            }
        }
        bool equal(const Iterator& other) const
        {
            return &m_graph == &other.m_graph && m_index == other.m_index && m_node == other.m_node;
        }
        NodeIterator m_node{};
        std::size_t m_index{};
        const G& m_graph;
    };

    Iterator begin() { return Iterator(m_graph, m_graph.m_graph.control.nodes.begin()); }
    Iterator end() { return Iterator(m_graph, m_graph.m_graph.control.nodes.end()); }


private:
    friend G;

    Endpoints(G& graph)
    : m_graph(graph)
    {
    }

    G& m_graph;
};

} // namespace ygglet::processor::graph
