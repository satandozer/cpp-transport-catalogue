#pragma once

#include <iosfwd>
#include <string_view>

#include "transport_catalogue.h"

namespace request{

    class Reader {
        public:

            Reader(std::ostream& output, transport::Catalogue& transport_catalogue) 
                : catalogue_(transport_catalogue)
                , output_(output){}

            void ReadAndPrintRequests (std::istream& input);

        private:

            transport::Catalogue& catalogue_;
            std::ostream& output_;

            void ParseAndPrint(std::string_view request) const;

            void PrintBus(std::string_view bus_name) const;

            void PrintStop(std::string_view stop_name) const;

    };


}