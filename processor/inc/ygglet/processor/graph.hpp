#pragma once

#include <ygglet/processor/render_graph.hpp>

#include <boost/container/flat_set.hpp>
#include <boost/uuid/uuid.hpp>

#include <array>
#include <atomic>
#include <vector>

namespace ygglet::processor {

struct Node;

struct Graph
{
    Graph(const Graph&) = delete;
    Graph(Graph&&) = delete;
    Graph& operator=(const Graph&) = delete;
    Graph& operator=(Graph&&) = delete;

    //
    // audio thread
    //

    void process();

    //
    // control threads
    //

    Graph();
    ~Graph();

    void compile();
    void cleanup();
    void insert(std::unique_ptr<Node>);
    bool connect(boost::uuids::uuid in, boost::uuids::uuid out);

    bool isConnected(boost::uuids::uuid in, boost::uuids::uuid out) const;
    bool contains(boost::uuids::uuid node) const;

private:
    using Nodes = std::vector<std::unique_ptr<Node>>;
    using Render = std::vector<Node*>;
    struct Connection
    {
        boost::uuids::uuid in{};
        boost::uuids::uuid out{};
    };
    using Connections = boost::container::flat_set<Connection>;

    Nodes m_nodes;
    Connections m_connections;
    struct
    {
        std::atomic<std::uint32_t> current{};
        std::array<Render, 2> buffers{};
    } m_render;
};

} // namespace ygglet::processor
