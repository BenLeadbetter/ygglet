#pragma once

#include <ygglet/engine/node.hpp>

#include <boost/iterator/iterator_facade.hpp>
#include <boost/uuid/uuid.hpp>

#include <memory>

namespace ygglet::engine {

template <typename G> struct Nodes
{
    struct iterator : public boost::iterator_facade<iterator, Node, boost::random_access_traversal_tag>
    {
        iterator() = default;

    private:
        friend class boost::iterator_core_access;
        friend struct Nodes;
        using UnderlyingIterator = decltype(std::declval<G&>().m_graph.control.nodes.begin());
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

    iterator begin() { return iterator(m_engine.m_graph.control.nodes.begin()); }
    iterator end() { return iterator(m_engine.m_graph.control.nodes.end()); }

    const_iterator begin() const { return begin(); }
    const_iterator end() const { return end(); }

    void insert(std::unique_ptr<Node> node)
        requires(!std::is_const_v<G>)
    {
        // allocate buffers
        auto& buffers = G::buffers(*node);
        buffers.resize(node->outputs().size());
        for (auto& buffer : buffers)
        {
            buffer = std::vector<float>(m_engine.m_blockSize, 0.0f);
        }

        auto id = node->id();
        m_engine.m_graph.control.nodes.insert({id, std::move(node)});
    }

    void remove(boost::uuids::uuid id)
        requires(!std::is_const_v<G>)
    {
        // TODO: safely mark for deletion and destroy
        // after an "audio epoch" when the render graph is
        // definitately not referencing it
    }

    Node& operator[](boost::uuids::uuid id)
        requires(!std::is_const_v<G>)
    {
        return *m_engine.m_graph.control.nodes[id];
    }

    const Node& operator[](boost::uuids::uuid id) const
    {
        auto itr = m_engine.m_graph.control.nodes.find(id);
        return *itr->second;
    }

    iterator find(boost::uuids::uuid id) { return iterator{m_engine.m_graph.control.nodes.find(id)}; }

private:
    friend G;

    Nodes(G& engine)
    : m_engine(engine)
    {
    }

    G& m_engine;
};

} // namespace ygglet::engine
