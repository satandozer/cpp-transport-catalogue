#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <iostream>
#include <tuple>
#include <iostream>

namespace input {
    namespace parse {
        //Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
        geo::Coordinates Coordinates(std::string_view str) {
            static const double nan = std::nan("");

            auto not_space = str.find_first_not_of(' ');
            auto comma = str.find(',');

            if (comma == str.npos) {
                return {nan, nan};
            }

            auto not_space2 = str.find_first_not_of(' ', comma + 1);

            double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
            double lng = std::stod(std::string(str.substr(not_space2)));

            return {lat, lng};
        }

        //Удаляет пробелы в начале и конце строки
        std::string_view Trim(std::string_view string) {
            const auto start = string.find_first_not_of(' ');
            if (start == string.npos) {
                return {};
            }
            return string.substr(start, string.find_last_not_of(' ') + 1 - start);
        }

        //Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
        std::vector<std::string_view> Split(std::string_view string, char delim) {
            std::vector<std::string_view> result;

            size_t pos = 0;
            while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
                auto delim_pos = string.find(delim, pos);
                if (delim_pos == string.npos) {
                    delim_pos = string.size();
                }
                if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
                    result.push_back(substr);
                }
                pos = delim_pos + 1;
            }

            return result;
        }

        /* Парсит маршрут.
        * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
        * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A] */
        std::vector<std::string_view> Route(std::string_view route) {
            if (route.find('>') != route.npos) {
                return Split(route, '>');
            }

            auto stops = Split(route, '-');
            std::vector<std::string_view> results(stops.begin(), stops.end());
            results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

            return results;
        }

        command::Description CommandDescription(std::string_view line) {
            auto colon_pos = line.find(':');
            if (colon_pos == line.npos) {
                return {};
            }

            auto space_pos = line.find(' ');
            if (space_pos >= colon_pos) {
                return {};
            }

            auto not_space = line.find_first_not_of(' ', space_pos);
            if (not_space >= colon_pos) {
                return {};
            }

            return {std::string(line.substr(0, space_pos)),
                    std::string(line.substr(not_space, colon_pos - not_space)),
                    std::string(line.substr(colon_pos + 1))};
        }
        
        //Парсит строку вида "Xm to Stop"
        std::pair<std::string_view,int> Distance (std::string_view line){
            auto not_space = line.find_first_not_of(' ');
            auto m_point = line.find('m');
            int distance = std::stoi(std::string(line.substr(not_space, m_point - not_space)));
            
            auto point = line.find('o');
            not_space = line.find_first_not_of(' ',point+1);
            point = line.find(',');
            std::string_view stop = line.substr(not_space,point-not_space);
            return std::make_pair(stop,distance);
        }

        //Парсит описание остановки
        command::StopDescription Stop (std::string_view line) {
            command::StopDescription this_stop;
            auto not_space = line.find_first_not_of(' ');
            auto comma1 = line.find(',');
            auto comma2 = line.find(',',comma1+1);
            this_stop.coordinates = parse::Coordinates(line.substr(not_space,comma2-not_space));
            if (comma2 == line.npos) {
                return this_stop;
            }
            std::size_t prev = comma2;
            std::size_t comma = prev;
            while (comma != line.npos){
                comma = line.find(',',prev+1);
                this_stop.distances.push_back(Distance(line.substr(prev+1,comma-prev)));
                prev = comma;
            }
            return this_stop;
            
        }
    
    }
    
    void Reader::ParseLine(std::string_view line) {
        auto command_description = parse::CommandDescription(line);
        if (command_description) {
            commands_.push_back(std::move(command_description));
        }
    }

    void Reader::ApplyCommands([[maybe_unused]] transport::Catalogue& catalogue) const {
        std::vector<std::pair<std::string,geo::Coordinates>> stop_commands;
        std::vector<std::pair<std::string,std::vector<std::string>>> bus_commands;
        std::map<std::pair<std::string,std::string>,int> distances;

        std::string stop_name;
        std::string bus_name;
        std::vector<std::string> bus_stops;
        

        for (auto command : commands_){
            if (command.command == "Stop"){
                command::StopDescription stop_description;
                stop_name = command.id;
                stop_description = parse::Stop(command.description);
                for (const auto& [stop,distance] : stop_description.distances){
                    distances[std::make_pair(stop_name,std::string(stop))] = distance;
                }
                stop_commands.push_back(std::make_pair(stop_name,stop_description.coordinates));
            } 
            else if (command.command == "Bus"){
                bus_name = command.id;
                for (std::string_view stop : parse::Route(command.description)){
                    bus_stops.push_back(std::string(stop));
                }
                bus_commands.push_back({bus_name,bus_stops});
                bus_stops.clear();
            }
        }

        for (auto [name,coordinates] : stop_commands){
            catalogue.AddStop(name,coordinates);
        }

        catalogue.AddDistances(distances);

        for (auto [name,stops] : bus_commands){
            catalogue.AddBus(name,stops);
        }
    }        
}