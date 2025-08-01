#include "Coin.h"
#include <algorithm>

bool Coin::Deserialize(const rapidjson::Value & obj)
{
  // In some cases the values can be null. Double seems to work for any number
  m_id = obj["id"].IsString() ? obj["id"].GetString() : m_id;
  m_name = obj["name"].IsString() ? obj["name"].GetString() : m_name;
  m_current_price = obj["current_price"].IsNumber() ? obj["current_price"].GetDouble() : m_current_price;
  m_market_cap = obj["market_cap"].IsNumber() ? obj["market_cap"].GetDouble() : m_market_cap;
  m_market_cap_rank = obj["market_cap_rank"].IsNumber() ? obj["market_cap_rank"].GetInt() : m_market_cap_rank;
  m_ath = obj["ath"].IsNumber() ? obj["ath"].GetDouble() : m_ath;
  m_ath_change_percentage = obj["ath_change_percentage"].IsNumber() ? obj["ath_change_percentage"].GetFloat() : m_ath_change_percentage;
  m_ath_date = obj["ath_date"].IsString() ? obj["ath_date"].GetString() : m_ath_date;
  m_atl = obj["atl"].IsNumber() ? obj["atl"].GetDouble() : m_atl;
  m_atl_change_percentage = obj["atl_change_percentage"].IsNumber() ? obj["atl_change_percentage"].GetFloat() : m_atl_change_percentage;
  m_atl_date = obj["atl_date"].IsString() ? obj["atl_date"].GetString() : m_atl_date;
  m_price_change_percentage_14d_in_currency = obj["price_change_percentage_14d_in_currency"].IsNumber() ? obj["price_change_percentage_14d_in_currency"].GetFloat() : m_price_change_percentage_14d_in_currency;
  m_price_change_percentage_1h_in_currency =  obj["price_change_percentage_1h_in_currency"].IsNumber()  ? obj["price_change_percentage_1h_in_currency"].GetFloat() : m_price_change_percentage_1h_in_currency;
  m_price_change_percentage_24h_in_currency = obj["price_change_percentage_24h_in_currency"].IsNumber() ? obj["price_change_percentage_24h_in_currency"].GetFloat() : m_price_change_percentage_24h_in_currency;
  m_price_change_percentage_30d_in_currency = obj["price_change_percentage_30d_in_currency"].IsNumber() ? obj["price_change_percentage_30d_in_currency"].GetFloat() : m_price_change_percentage_30d_in_currency;
  m_price_change_percentage_7d_in_currency =  obj["price_change_percentage_7d_in_currency"].IsNumber()  ? obj["price_change_percentage_7d_in_currency"].GetFloat() : m_price_change_percentage_7d_in_currency;

  // TODO investigate if it would be possible or worth it to print emoticons as well instead of "?" It is possible that additional settings would need to be enabled in Imgui. Maybe at least monochrome versions

  return true;
}