#pragma once

#include <string>
#include <vector>
#include <deque>
#include <unordered_map>
#include <optional>
#include <variant>
#include <map>
#include "domain.h"

namespace transport {

	using domain::Stop;
	using domain::Bus;
	using domain::StopPairHasher;
	using domain::PairHasher;
	using domain::DistancesStopMap;
	using domain::DistancesStringMap;

	class Catalogue {

		public:

		void AddStop(Stop&& stop);
		
		Bus* AddBus(Bus&& bus);

		void AddDistances(const DistancesStringMap& distances );

		std::optional<Stop*> GetStop(const std::string& stop_name) const;

		std::optional<Bus*> GetBus(const std::string& bus_name) const;

		std::optional<double> GetDistance(const std::string& first, const std::string& second) const;

		std::vector<Bus*> GetAllBus() const;

		std::vector<Stop*> GetAllStops() const; 

		private:
		
		std::deque<Stop> stops_;
		std::map<std::string_view,Stop*> stops_id_;
		
		std::deque<Bus> buses_;
		std::map<std::string_view,Bus*> buses_id_;

		DistancesStopMap distances_;
	};

}

