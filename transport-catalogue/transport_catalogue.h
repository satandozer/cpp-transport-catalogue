#pragma once

#include <string>
#include <vector>
#include <deque>
#include <unordered_map>
#include <optional>
#include <set>
#include "geo.h"

namespace transport {
	namespace data {
			struct Stop {
	
				Stop(const std::string& name_, const geo::Coordinates& coordinates_)
					: name(name_)
					, coordinates(coordinates_){}	

				Stop(const std::string& name_, const double& latitude_, const double& longitude_)
					: name(name_){
						coordinates.lat = latitude_;
						coordinates.lng = longitude_;
					}

				std::string name = "";
				geo::Coordinates coordinates;
				std::set<std::string_view> buses;
			};

			struct Bus {

				std::string name = "";
				std::vector<Stop*> stops;
				int stops_count = 0;
				int unique_stops = 0;
				double length = 0.0;	
			};				
	}

	class Catalogue {	
		public:

		void AddStop(const std::string& name, const geo::Coordinates& coordinates_);
		
		void AddBus(const std::string& name, const std::vector<std::string>& stops);

		std::optional<data::Stop*> GetStop(const std::string& stop_name) const;

		std::optional<data::Bus*> GetBus(const std::string& bus_name) const;

		private:
		
		std::deque<data::Stop> stops_;
		std::unordered_map<std::string_view,data::Stop*> stops_id_;
		
		std::deque<data::Bus> buses_;
		std::unordered_map<std::string,data::Bus*> buses_id_;

	};
}

