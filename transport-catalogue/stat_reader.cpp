#include <iomanip>
#include "stat_reader.h"

namespace stat_reader {

    void ParseAndPrint(const transport::Catalogue& transport_catalogue, std::string_view request,
                        std::ostream& output) {
        auto it = request.find(' ');
        std::string_view request_type = request.substr(0, it);
        if (std::string(request_type) == "Bus"){
            auto it2 = request.find_first_not_of(' ',it+1);
            std::string_view bus_name = request.substr(it2,request.npos);
            output << std::setprecision(6) << "Bus " << bus_name << ": ";

            if(transport_catalogue.GetBus(std::string(bus_name))!=nullptr){
                int stops = transport_catalogue.GetBus(std::string(bus_name)).value()->stops_count;
                int unique = transport_catalogue.GetBus(std::string(bus_name)).value()->unique_stops;
                double length = transport_catalogue.GetBus(std::string(bus_name)).value()->length;

                output << stops << " stops on route, "
                        << unique << " unique stops, "
                        << length << " route length" << std::endl;
            }
            else {
                output << "not found" << std::endl;
            }
        } else if (std::string(request_type) == "Stop"){
            auto it2 = request.find_first_not_of(' ',it+1);
            std::string_view stop_name = request.substr(it2,request.npos);
            output << "Stop " << stop_name << ": ";
            if (transport_catalogue.GetStop(std::string(stop_name))!=nullptr){
                auto buses = transport_catalogue.GetStop(std::string(stop_name)).value()->buses;
                if (buses.size()==0){
                    output << "no buses";
                }else {
                    output << "buses";
                    for (auto bus : buses){
                        output << ' ' << bus;
                    }
                }
                output << std::endl;
            }
            else {
                output << "not found" << std::endl;
            }
        }
    }

}

