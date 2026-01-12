#pragma once

#include <ygglet/engine/connection.hpp>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/uuid/generators.hpp>
#include <boost/uuid/uuid.hpp>

#include <tl/expected.hpp>

namespace ygglet::engine::connection_error {
struct NonExistentPort
{
    Connection::Port port;
    friend bool operator==(const NonExistentPort& lhs, const NonExistentPort& rhs) { return lhs.port == rhs.port; }
};
struct FeedbackLoop
{
    Connection connection;
    friend bool operator==(const FeedbackLoop& lhs, const FeedbackLoop& rhs)
    {
        return lhs.connection == rhs.connection;
    }
};
struct AlreadyConnected
{
    boost::uuids::uuid existing;
    friend bool operator==(const AlreadyConnected& lhs, const AlreadyConnected& rhs)
    {
        return lhs.existing == rhs.existing;
    }
};
using Error = std::variant<NonExistentPort, FeedbackLoop, AlreadyConnected>;
} // namespace ygglet::engine::connection_error

namespace ygglet::engine::detail {

template <typename E> struct Connections
{
    struct iterator : public boost::iterator_facade<iterator, std::pair<boost::uuids::uuid, Connection>,
                                                    boost::random_access_traversal_tag>
    {
        iterator() = default;

    private:
        friend class boost::iterator_core_access;
        friend struct Connections;
        using UnderlyingIterator = decltype(std::declval<E&>().m_control.connections.begin());
        iterator(UnderlyingIterator itr)
        : m_itr(itr)
        {
        }
        std::pair<boost::uuids::uuid, Connection>& dereference() const { return *m_itr; }
        void increment() { ++m_itr; }
        void decrement() { --m_itr; }
        void advance(std::ptrdiff_t n) { m_itr += n; }
        std::ptrdiff_t distance_to(const iterator& other) const { return other.m_itr - m_itr; }
        bool equal(const iterator& other) const { return m_itr == other.m_itr; }
        UnderlyingIterator m_itr;
    };

    using const_iterator = iterator;

    iterator begin() { return iterator(m_engine.m_control.connections.begin()); }
    iterator end() { return iterator(m_engine.m_control.connections.end()); }

    const_iterator begin() const { return iterator(m_engine.m_control.connections.begin()); }
    const_iterator end() const { return iterator(m_engine.m_control.connections.end()); }

    iterator find(boost::uuids::uuid id) { return m_engine.m_control.connections.find(id); }

    // TODO: transitions
    [[nodiscard]] tl::expected<boost::uuids::uuid, connection_error::Error> insert(const Connection& connection,
                                                                                   bool force = false)
        requires(!std::is_const_v<E>)
    {
        auto in = m_engine.nodes().find(connection.in.node);
        auto out = m_engine.nodes().find(connection.out.node);
        if (in == m_engine.nodes().end())
        {
            return tl::unexpected{connection_error::NonExistentPort{
                .port = connection.in,
            }};
        }
        if (out == m_engine.nodes().end())
        {
            return tl::unexpected{connection_error::NonExistentPort{
                .port = connection.out,
            }};
        }
        if (connection.in.port >= in->inputs())
        {
            return tl::unexpected{connection_error::NonExistentPort{
                .port = connection.in,
            }};
        }
        if (connection.out.port >= out->outputs())
        {
            return tl::unexpected{connection_error::NonExistentPort{
                .port = connection.out,
            }};
        }

        auto& descriptors = m_engine.m_control.descriptors;
        auto& graph = m_engine.m_control.graph;
        auto& connections = m_engine.m_control.connections;
        boost::uuids::uuid id = boost::uuids::random_generator{}();
        const auto outv = descriptors.find(connection.out.node)->second;
        const auto inv = descriptors.find(connection.in.node)->second;

        if (boost::is_reachable(
                inv, outv, graph,
                boost::make_vector_property_map<boost::default_color_type>(boost::get(boost::vertex_index, graph))))
        {
            return tl::unexpected{connection_error::FeedbackLoop{
                .connection = connection,
            }};
        }

        if (auto itr = std::find_if(
                begin(), end(),
                [&](const auto& p) {
                    return (p.second.in.node == connection.in.node && p.second.in.port == connection.in.port) ||
                           (p.second.out.node == connection.out.node && p.second.out.port == connection.out.port);
                });
            itr != end())
        {
            if (force)
            {
                // remove old connection
                const auto eo = descriptors.find(itr->second.out.node)->second;
                const auto ei = descriptors.find(itr->second.in.node)->second;
                boost::remove_edge(eo, ei, graph);
                connections.erase(itr->first);
            }
            else
            {
                return tl::unexpected{connection_error::AlreadyConnected{
                    .existing = itr->first,
                }};
            }
        }

        m_engine.m_control.connections.insert({id, connection});
        boost::add_edge(outv, inv, connection, graph);
        return id;
    }

    void disconnect(boost::uuids::uuid node)
    {
        const auto next = [this, &node](auto itr) {
            return std::find_if(itr, end(), [&](const auto& p) {
                return (p.second.in.node == node) || (p.second.out.node == node);
            });
        };
        auto& descriptors = m_engine.m_control.descriptors;
        auto& graph = m_engine.m_control.graph;
        auto& connections = m_engine.m_control.connections;

        for (auto itr = next(begin()); itr != end(); next(itr))
        {
            const auto eo = descriptors.find(itr->second.out.node)->second;
            const auto ei = descriptors.find(itr->second.in.node)->second;
            boost::remove_edge(eo, ei, graph);
            itr = connections.erase(itr.m_itr);
        }
    }

    // TODO: transitions
    void remove(boost::uuids::uuid id)
        requires(!std::is_const_v<E>)
    {
        m_engine.m_control.connections.erase(id);
    }

    std::size_t size() const { return m_engine.m_control.connections.size(); }

    bool empty() const { return m_engine.m_control.connections.empty(); }

private:
    friend E;

    Connections(E& engine)
    : m_engine(engine)
    {
    }

    E& m_engine;
};

} // namespace ygglet::engine::detail
