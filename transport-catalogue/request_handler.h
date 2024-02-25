#pragma once

#include "transport_catalogue.h"
#include "domain.h"
#include "json_reader.h"
#include "map_renderer.h"

#include <map>


namespace request {
    class Handler {
    public:

        Handler(transport::Catalogue& catalogue)
            : catalogue_(&catalogue)
            , renderer_(new renderer::MapRenderer(catalogue))
            , json_reader_(new json_reader::JsonReader(catalogue)) {
                json_reader_->SetRenderer(*renderer_);
            }

        void ReadInput(std::istream& input);
        void PrintJson(std::ostream& output) const;
        void RenderMap(std::ostream& output) const;  
        void GenerateOutput(std::ostream& json_output, std::ostream& svg_output) const;

    private:
        transport::Catalogue* catalogue_;
        renderer::MapRenderer* renderer_;
        json_reader::JsonReader* json_reader_;
        
    };
}


