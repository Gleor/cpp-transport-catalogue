#pragma once

#include "ranges.h"

#include <cstdlib>
#include <vector>

namespace graph {

    using VertexId = size_t;
    using EdgeId = size_t;

    enum EdgeType {
        wait, move
    };

    template <typename Weight>
    struct Edge {

        Edge& SetEdgeType(EdgeType ty) {
            type = ty;
            return *this;
        }
        Edge& SetVertexFromId(VertexId fr) {
            from = fr;
            return *this;
        }
        Edge& SetVertexToId(VertexId t) {
            to = t;
            return *this;
        }
        Edge& SetEdgeWeight(Weight w) {
            weight = w;
            return *this;
        }
        Edge& SetEdgeName(std::string name) {
            edge_name = std::move(name);
            return *this;
        }
        Edge& SetEdgeSpanCount(int count) {
            span_count = count;
            return *this;
        }

        EdgeType GetEdgeType() const {
            return type;
        }
        VertexId GetVertexFromId() const {
            return from;
        }
        VertexId GetVertexToId() const {
            return to;
        }
        Weight GetEdgeWeight() const {
            return weight;
        }
        std::string_view GetEdgeName() const {
            return edge_name;
        }
        int GetEdgeSpanCount() const {
            return span_count;
        }

        EdgeType type;
        VertexId from;
        VertexId to;
        Weight weight;
        std::string edge_name = {};
        int span_count = 0;
    };

    template <typename Weight>
    class DirectedWeightedGraph {
    private:
        using IncidenceList = std::vector<EdgeId>;
        using IncidentEdgesRange = ranges::Range<typename IncidenceList::const_iterator>;

    public:
        DirectedWeightedGraph() = default;
        explicit DirectedWeightedGraph(size_t vertex_count);
        EdgeId AddEdge(const Edge<Weight>& edge);

        size_t GetVertexCount() const;
        size_t GetEdgeCount() const;
        const Edge<Weight>& GetEdge(EdgeId edge_id) const;
        IncidentEdgesRange GetIncidentEdges(VertexId vertex) const;

    private:
        std::vector<Edge<Weight>> edges_;
        std::vector<IncidenceList> incidence_lists_;
    };

    template <typename Weight>
    DirectedWeightedGraph<Weight>::DirectedWeightedGraph(size_t vertex_count)
        : incidence_lists_(vertex_count) {
    }

    template <typename Weight>
    EdgeId DirectedWeightedGraph<Weight>::AddEdge(const Edge<Weight>& edge) {
        edges_.push_back(edge);
        const EdgeId id = edges_.size() - 1;
        incidence_lists_.at(edge.from).push_back(id);
        return id;
    }

    template <typename Weight>
    size_t DirectedWeightedGraph<Weight>::GetVertexCount() const {
        return incidence_lists_.size();
    }

    template <typename Weight>
    size_t DirectedWeightedGraph<Weight>::GetEdgeCount() const {
        return edges_.size();
    }

    template <typename Weight>
    const Edge<Weight>& DirectedWeightedGraph<Weight>::GetEdge(EdgeId edge_id) const {
        return edges_.at(edge_id);
    }

    template <typename Weight>
    typename DirectedWeightedGraph<Weight>::IncidentEdgesRange
        DirectedWeightedGraph<Weight>::GetIncidentEdges(VertexId vertex) const {
        return ranges::AsRange(incidence_lists_.at(vertex));
    }
}  // namespace graph