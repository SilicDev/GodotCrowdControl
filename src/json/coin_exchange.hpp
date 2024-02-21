#ifndef CROWD_CONTROL_COIN_EXCHANGE_HPP
#define CROWD_CONTROL_COIN_EXCHANGE_HPP

#include "json/user.hpp"
#include "util/json_serializer.hpp"

using namespace godot;

struct CoinExchange {
	int amount = 0;
	String coin_type;
	User target;
	User exchanger;
	uint64_t stamp = 1;

	constexpr static auto json_properties = std::make_tuple(
			json_property(&CoinExchange::amount, "amount"),
			json_property(&CoinExchange::coin_type, "coinType"),
			json_property(&CoinExchange::target, "target"),
			json_property(&CoinExchange::exchanger, "exchanger"),
			json_property(&CoinExchange::stamp, "stamp"));
};

#endif // CROWD_CONTROL_COIN_EXCHANGE_HPP