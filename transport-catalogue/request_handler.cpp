#include "request_handler.h"

namespace request {
    void Handler::ReadInput(std::istream& input) {
        json_reader_->ParseJson(input);
    }
    void Handler::PrintJson(std::ostream& output) const{
        json_reader_->PrintJson(output);
    }
    void Handler::RenderMap(std::ostream& output) const{
        renderer_->RenderMap(output);
    }
    void Handler::GenerateOutput(std::ostream& json_output, std::ostream& svg_output) const{
        PrintJson(json_output);
        RenderMap(svg_output);
    }
}