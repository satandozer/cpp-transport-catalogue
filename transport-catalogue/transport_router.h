#pragma once

#include "transport_catalogue.h"
#include "router.h"
#include "log_duration.h"

#include <numeric>

namespace transport_router {
    using namespace domain;
    using namespace router_data;
    using namespace graph;

    class TransportRouter{

        public:

            explicit TransportRouter(transport::Catalogue& catalogue) 
                : catalogue_(catalogue) {}

            void SetSettings(Settings settings){
                settings_ = settings;
            }

            void LoadCatalogue();

            std::optional<Response> GetRoute (Stop* start, Stop* end) const;

        private:
            //Basic setup
            transport::Catalogue& catalogue_;
            
            //Contents filled lately
            Settings settings_;
            VertexId last_id_ = 0;
            std::unordered_map<Stop*,StopVertexPair> stops_;
            std::unordered_map<EdgeId,EdgeData> edges_;
            DirectedWeightedGraph<Time>* graph_;
            Router<Time>* router_;

            void AddStop (Stop* stop);
            void AddBus (Bus* bus);
    };

}