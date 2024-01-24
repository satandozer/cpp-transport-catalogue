#pragma once

#include <string>
#include <vector>
#include <deque>
#include <unordered_map>
#include <optional>
#include <set>
#include <map>
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
				int length = 0;	
				double geo_length = 0.0;
				double curvature = 0.0;
			};

		struct StopPairHasher {
			public:
				size_t operator()(const std::pair<data::Stop*,data::Stop*>& stop_pair)const{
					std::hash<std::string> hash_;
					std::size_t hash1 = hash_(stop_pair.first->name);
					std::size_t hash2 = hash_(stop_pair.second->name);
					return (hash1 + 11*hash2);
				}
		};
	}

	class Catalogue {	
		public:

		void AddStop(const std::string& name, const geo::Coordinates& coordinates_);
		
		void AddBus(const std::string& name, const std::vector<std::string>& stops);

		void AddDistances(const std::map<std::pair<std::string,std::string>,int>& distances);

		std::optional<data::Stop*> GetStop(const std::string& stop_name) const;

		std::optional<data::Bus*> GetBus(const std::string& bus_name) const;

		private:
		
		std::deque<data::Stop> stops_;
		std::unordered_map<std::string_view,data::Stop*> stops_id_;
		
		std::deque<data::Bus> buses_;
		std::unordered_map<std::string,data::Bus*> buses_id_;

		std::unordered_map<std::pair<data::Stop*,data::Stop*>,int,data::StopPairHasher> distances_;
	};
}

