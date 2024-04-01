#include "transport_router.h"

namespace transport_router{
    void TransportRouter::LoadCatalogue(){
        graph_ = new graph::DirectedWeightedGraph<Time>(catalogue_.GetAllStops().size()*2);
        for (auto stop : catalogue_.GetAllStops()){
            AddStop(stop);
        }
        for (auto bus : catalogue_.GetAllBus()){
            AddBus(bus);
        }
        router_ = new Router<Time>(*graph_);
    }

    std::optional<Response> TransportRouter::GetRoute (Stop* start, Stop* end) const {
        std::optional<Router<Time>::RouteInfo> info 
            = router_->BuildRoute(stops_.at(start).stop_begin.id,stops_.at(end).stop_begin.id);
        if (!info.has_value()) {
            return std::nullopt;
        }
        Response response;
        response.total_time = info.value().weight;
        for (auto edge : info.value().edges){
            EdgeData data = edges_.at(edge);
            ResponseItem item;
            item.type = data.type;
            if (data.type == EdgeType::WAIT){
                item.name = data.stop_begin->name;
                item.time = settings_.bus_wait_time;
            } else if (data.type == EdgeType::BUS){
                item.name = data.bus->name;
                item.span_count = data.span_count;
                item.time = graph_->GetEdge(edge).weight;
            }
            response.items.push_back(item);
        }
        return response;
    }

    void TransportRouter::AddStop (Stop* stop){
        StopVertex new_start {last_id_,stop};
        StopVertex new_finsh {++last_id_,stop};
        ++last_id_;
        stops_[stop] = StopVertexPair{new_start,new_finsh};

        Edge<Time> new_edge {
            new_start.id,
            new_finsh.id,
            static_cast<double>(settings_.bus_wait_time)
        };

        EdgeId id = graph_->AddEdge(new_edge);
        EdgeData new_edge_data {EdgeType::WAIT};
        new_edge_data.stop_begin = stop;
        new_edge_data.stop_end = stop;
        edges_[id] = new_edge_data;
    }

    void TransportRouter::AddBus (Bus* bus){
        auto it_begin = bus->stops.begin();
        auto it_end = bus->stops.end();
        for (auto it_from = std::next(it_end,-1); it_from >= it_begin; --it_from){
            auto it_prev = it_from;
            double dist_prev = 0;
            double rev_dist_prev = 0;
            for (auto it_to = std::next(it_from); it_to != it_end; ++it_to){
                Edge<Time> new_edge;
                new_edge.from = stops_.at(*it_from).stop_end.id;
                new_edge.to = stops_.at(*it_to).stop_begin.id;
                int span_count = static_cast<int>(std::distance(it_from,it_to));
                double distance = dist_prev + catalogue_.GetDistance((*it_prev)->name,(*it_to)->name).value();
                dist_prev = distance;
                double rev_distance = 0;

                Edge<Time> new_edge_reverse;
                
                if (!bus->is_roundtrip){
                    new_edge_reverse.from = stops_.at(*it_to).stop_end.id;
                    new_edge_reverse.to = stops_.at(*it_from).stop_begin.id;
                    rev_distance = rev_dist_prev + catalogue_.GetDistance((*it_to)->name,(*it_prev)->name).value();
                    rev_dist_prev = rev_distance;
                }
                it_prev = it_to;

                new_edge.weight = ((distance/1000.0)/settings_.velocity)*60.0;
                new_edge_reverse.weight = ((rev_distance/1000.0)/settings_.velocity)*60.0;

                EdgeId id = graph_->AddEdge(new_edge);
                EdgeData edge_data {EdgeType::BUS,span_count,bus,*it_from,*it_to};
                edges_[id] = edge_data;
                if (!bus->is_roundtrip){
                    EdgeId rev_id = graph_->AddEdge(new_edge_reverse);
                    edges_[rev_id] = edge_data;
                }
                
            }
        }
    }
              
}