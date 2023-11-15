#pragma optimize("", off)

#include "Coin.h"
#include "Logging.h"
#include <algorithm>
#include <iostream>

bool Coin::Serialize(rapidjson::Writer<rapidjson::StringBuffer> * writer) const
{
  return false;
}

const char* ValueOrEmpty(const char* s)
{
  return s == nullptr ? "" : s;
}

bool Coin::Deserialize(const rapidjson::Value & obj)
{
  m_id =  obj["id"].GetString(); 
  m_name = obj["name"].GetString();
  m_current_price = obj["current_price"].GetFloat();
  m_market_cap = obj["market_cap"].GetFloat();
  m_market_cap_rank = obj["market_cap_rank"].GetInt();
  m_ath = obj["ath"].GetFloat();
  m_ath_change_percentage = obj["ath_change_percentage"].GetFloat();
  m_ath_date = ValueOrEmpty(obj["ath_date"].GetString());
  m_atl = obj["atl"].GetFloat();
  m_atl_change_percentage = obj["atl_change_percentage"].GetFloat();
  m_atl_date = ValueOrEmpty(obj["atl_date"].GetString());
  m_price_change_percentage_14d_in_currency = obj["price_change_percentage_14d_in_currency"].GetFloat();
  m_price_change_percentage_1h_in_currency = obj["price_change_percentage_1h_in_currency"].GetFloat();
  m_price_change_percentage_24h_in_currency = obj["price_change_percentage_24h_in_currency"].GetFloat();
  m_price_change_percentage_30d_in_currency = obj["price_change_percentage_30d_in_currency"].GetFloat();
  m_price_change_percentage_7d_in_currency = obj["price_change_percentage_7d_in_currency"].GetFloat();

  std::replace(m_name.begin(), m_name.end(), '%', '^'); // replace all '%' to '^' to avoid crash when printing messages

  return true;
}

void Coin::PrintInfo()
{
  std::cout <<
    " id = " << m_id <<
    "\n name = " << m_name <<
    "\n current_price = " << m_current_price <<
    "\n market_cap = " << m_market_cap <<
    "\n market_cap_rank = " << m_market_cap_rank <<
    "\n ath = " << m_ath <<
    "\n ath_change_percentage = " << m_ath_change_percentage <<
    "\n ath_date = " << m_ath_date <<
    "\n atl = " << m_atl <<
    "\n atl_change_percentage = " << m_atl_change_percentage <<
    "\n atl_date = " << m_atl_date <<
    "\n price_change_percentage_14d_in_currency = " << m_price_change_percentage_14d_in_currency <<
    "\n price_change_percentage_24h_in_currency = " << m_price_change_percentage_24h_in_currency <<
    "\n price_change_percentage_7d_in_currency = " << m_price_change_percentage_7d_in_currency << std::endl;
}

//Coin::Platform Coin::GetPlatform()
//{
  //if (m_platform == Platform::UNSET)
  //{
  //  gecko::api coinGecko;
  //  if (!coinGecko.ping())
  //  {
  //    ReportError("CoinGecko is offline! Failed to get coin's platform.");
  //    return m_platform;
  //  }

  //  gecko::coinsFunctions coins;
  //  gecko::web::response response;

  //  response = coins.getCoinData(m_id, false, false, false, false, false, false);

  //  rapidjson::Document doc;
  //  if (!InitDocument(response.text, doc))
  //  {
  //    ReportError("Failed to open the document in order to get coin's platform.");
  //    return m_platform;
  //  }

  //  std::string platform = ValueOrEmpty(doc["asset_platform_id"].GetString());
  //  if (platform == "ethereum")
  //  {
  //    m_platform = Platform::ETHEREUM;
  //    m_buy_link = "https://app.uniswap.org/#/swap?outputCurrency=";
  //  }
  //  else if (platform == "binance-smart-chain")
  //  {
  //    m_platform = Platform::BINANCE;
  //    m_buy_link = "https://pancakeswap.finance/swap?outputCurrency=";
  //  }
  //  else if (platform == "")
  //  {
  //    m_platform = Platform::STANDALONE;
  //  }
  //}

  //return m_platform;
//}