#pragma once

#include <iosfwd>
#include <string_view>

#include "transport_catalogue.h"

namespace stat_reader{
    void ParseAndPrint(const transport::Catalogue& tansport_catalogue, std::string_view request,
                       std::ostream& output);
}