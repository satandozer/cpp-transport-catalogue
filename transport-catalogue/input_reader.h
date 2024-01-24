#pragma once
#include <string>
#include <string_view>
#include <vector>

#include "geo.h"
#include "transport_catalogue.h"

namespace input{
    namespace command{
        struct Description {
            // Определяет, задана ли команда (поле command непустое)
            explicit operator bool() const {
                return !command.empty();
            }

            bool operator!() const {
                return !operator bool();
            }

            std::string command;      // Название команды
            std::string id;           // id маршрута или остановки
            std::string description;  // Параметры команды
        };

        struct StopDescription {
            geo::Coordinates coordinates;
            std::vector<std::pair<std::string_view,int>> distances;
        };
    }

    class Reader {
    public:

        //Парсит строку в структуру CommandDescription и сохраняет результат в commands_
        void ParseLine(std::string_view line);

        //Наполняет данными транспортный справочник, используя команды из commands_
        void ApplyCommands(transport::Catalogue& catalogue) const;

    private:
        std::vector<command::Description> commands_;
    };
}



