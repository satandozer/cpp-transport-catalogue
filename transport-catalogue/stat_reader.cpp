#include <iomanip>
#include "stat_reader.h"

namespace request {

    void Reader::ReadAndPrintRequests(std::istream& input) {
        int stat_request_count;
        input >> stat_request_count >> std::ws;
        for (int i = 0; i < stat_request_count; ++i) {
            std::string line;
            getline(input, line);
            ParseAndPrint(line);
        }
    }

    void Reader::ParseAndPrint(std::string_view request) const {
        auto it = request.find(' ');
        std::string_view request_type = request.substr(0, it);
        if (std::string(request_type) == "Bus"){
            auto it2 = request.find_first_not_of(' ',it+1);
            std::string_view bus_name = request.substr(it2,request.npos);
            PrintBus(bus_name);

        } else if (std::string(request_type) == "Stop"){
            auto it2 = request.find_first_not_of(' ',it+1);
            std::string_view stop_name = request.substr(it2,request.npos);
            PrintStop(stop_name);
        }
    }

    void Reader::PrintBus(std::string_view bus_name) const {
        output_ << std::setprecision(6) << "Bus " << bus_name << ": ";

        if(catalogue_.GetBus(std::string(bus_name))!=nullptr){
            int stops = catalogue_.GetBus(std::string(bus_name)).value()->stops_count;
            int unique = catalogue_.GetBus(std::string(bus_name)).value()->unique_stops;
            double length = catalogue_.GetBus(std::string(bus_name)).value()->length;
            double curvature = catalogue_.GetBus(std::string(bus_name)).value()->curvature;

            output_ << stops << " stops on route, "
                    << unique << " unique stops, "
                    << length << " route length, " 
                    << curvature << " curvature" << std::endl;
        }
        else {
            output_ << "not found" << std::endl;
        }
    }

    void Reader::PrintStop(std::string_view stop_name) const {
        output_ << "Stop " << stop_name << ": ";
        if (catalogue_.GetStop(std::string(stop_name))!=nullptr){
            auto buses = catalogue_.GetStop(std::string(stop_name)).value()->buses;
            if (buses.size()==0){
                output_ << "no buses";
            }else {
                output_ << "buses";
                for (auto bus : buses){
                    output_ << ' ' << bus;
                }
            }
            output_ << std::endl;
        }
        else {
            output_ << "not found" << std::endl;
        }
    }

}

