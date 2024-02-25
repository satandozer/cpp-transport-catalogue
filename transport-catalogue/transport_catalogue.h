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

	class Catalogue {

		public:

		void AddStop(Stop&& stop);
		
		Bus* AddBus(Bus&& bus);

		void AddDistances(const std::unordered_map<std::pair<std::string,std::string>,double,PairHasher>& distances );

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

		std::unordered_map<std::pair<Stop*,Stop*>,double,StopPairHasher> distances_;
	};

	class RequestProcessor {
		public:
			void SetCatalogue(Catalogue& catalogue);
			void AddStopCommand(const std::string& name, 
								const geo::Coordinates& coordinates_, 
								const std::vector<std::pair<std::string,double>>& distances);
			void AddBusCommand(const std::string& name, bool is_roundtrip, const std::vector<std::string>& stops);
			void AddRequest (domain::request::Command);
			void FillCatalogue();
		protected:
			std::vector<domain::request::Response> GetRequests();
		private:
			Catalogue* catalogue_;
			std::vector<Stop> stop_commands_;
			std::vector<domain::command::BusDescription> bus_commands_;
			std::unordered_map<std::pair<std::string,std::string>,double,PairHasher> distances_;
			std::vector<domain::request::Command> requests_;
	};
}

