#pragma once

#include "transport_catalogue.h"
#include "domain.h"
#include "json_reader.h"
#include "map_renderer.h"

#include <map>


namespace request {

	using domain::Stop;
	using domain::Bus;
	using domain::StopPairHasher;
	using domain::PairHasher;
    using domain::DistancesStringMap;
    using domain::DistancesStopMap;
    using domain::ParsedInput;
    using transport::Catalogue;
    using json_reader::JsonReader;
    using renderer::MapRenderer;


    class Handler {
    public:
        Handler(Catalogue& catalogue)
            : catalogue_(&catalogue)
            , renderer_(new MapRenderer(catalogue))
            , json_reader_(new JsonReader(catalogue)) {
                json_reader_->SetRenderer(*renderer_);
            }

        void ReadJson(std::istream& input);
        void PrintJson(std::ostream& output) const;
        void RenderMap(std::ostream& output) const;  
        void GenerateOutput(std::ostream& json_output, std::ostream& svg_output) const;

        void FillCatalogue();  

    private:
        Catalogue* catalogue_;
        MapRenderer* renderer_;
        JsonReader* json_reader_;
        ParsedInput commands_;
        
        std::vector<domain::request::Response> GetRequests() const; 
    };

}


