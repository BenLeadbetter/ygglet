#pragma once

#include <ygglet/engine/detail/endpoint.hpp>
#include <ygglet/engine/node.hpp>

#include <boost/assert.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/iterator/iterator_facade.hpp>

#include <memory>

namespace ygglet::engine::detail {

template <typename E> struct Nodes
{
    struct iterator : public boost::iterator_facade<iterator, Node, boost::random_access_traversal_tag>
    {
        iterator() = default;

    private:
        friend class boost::iterator_core_access;
        friend struct Nodes;
        using UnderlyingIterator = decltype(std::declval<E&>().m_control.nodes.begin());
        iterator(UnderlyingIterator itr)
        : m_itr(itr)
        {
        }
        Node& dereference() const { return *m_itr->second; }
        void increment() { ++m_itr; }
        void decrement() { --m_itr; }
        void advance(std::ptrdiff_t n) { m_itr += n; }

        std::ptrdiff_t distance_to(const iterator& other) const { return other.m_itr - m_itr; }
        bool equal(const iterator& other) const { return m_itr == other.m_itr; }
        UnderlyingIterator m_itr;
    };

    using const_iterator = iterator;

    const Node& input() { return *m_engine.m_control.input; }
    const Node& output() { return *m_engine.m_control.output; }

    iterator begin() { return iterator(m_engine.m_control.nodes.begin()); }
    iterator end() { return iterator(m_engine.m_control.nodes.end()); }

    const_iterator begin() const { return iterator(m_engine.m_control.nodes.begin()); }
    const_iterator end() const { return iterator(m_engine.m_control.nodes.end()); }

    void insert(std::unique_ptr<Node> node)
        requires(!std::is_const_v<E>)
    {
        auto id = node->id();
        auto descriptor = boost::add_vertex(id, m_engine.m_control.graph);
        m_engine.m_control.descriptors.insert({id, descriptor});
        m_engine.m_control.nodes.insert({id, std::move(node)});
    }

    void remove(NodeId id)
        requires(!std::is_const_v<E>)
    {
        // TODO: safely mark for deletion and destroy
        // after an "audio epoch" when the render graph is
        // definitately not referencing it
    }

    Node& operator[](NodeId id)
        requires(!std::is_const_v<E>)
    {
        return *m_engine.m_control.nodes[id];
    }

    const Node& operator[](NodeId id) const
    {
        auto itr = m_engine.m_control.nodes.find(id);
        return *itr->second;
    }

    iterator find(NodeId id) { return iterator{m_engine.m_control.nodes.find(id)}; }

    std::size_t size() const { return m_engine.m_control.nodes.size(); }

    bool empty() const { return m_engine.m_control.nodes.empty(); }

private:
    friend E;

    Nodes(E& engine)
    : m_engine(engine)
    {
    }

    E& m_engine;
};

} // namespace ygglet::engine::detail
