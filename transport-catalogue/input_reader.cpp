#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <iostream>
#include <tuple>

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
        
        std::string stop_name;
        geo::Coordinates stop_coordinates;
        std::string bus_name;
        std::vector<std::string> bus_stops;
        for (auto command : commands_){
            if (command.command == "Stop"){
                stop_name = command.id;
                stop_coordinates = parse::Coordinates(command.description);
                stop_commands.push_back({stop_name,stop_coordinates});
            } else if (command.command == "Bus"){
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

        for (auto [name,stops] : bus_commands){
            catalogue.AddBus(name,stops);
        }
    }        
}