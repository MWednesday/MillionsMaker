#pragma once

#include "JSONBase.h"
#include <map>

struct Coin : public JSONBase
{
public:
  virtual bool Deserialize(const rapidjson::Value& obj);
  virtual bool Serialize(rapidjson::Writer<rapidjson::StringBuffer>* writer) const;

  virtual void PrintInfo();

  inline bool operator< (const Coin& rhs) const { return m_id.compare(rhs.m_id) < 0; }
  inline bool operator< (const std::string& rhs) const { return m_id.compare(rhs) < 0; }

  enum class Platform
  {
    UNSET               = 0,      // Platform not initialized yet
    UNKNOWN             = 1 << 0, // Such platform wasn't added into this enum yet. Something new/unpopular
    ETHEREUM            = 1 << 1,
    BINANCE_SMART_CHAIN = 1 << 2,
    POLYGON_POS         = 1 << 3,
    HARMONY_SHARD_0     = 1 << 4,
    SOLANA              = 1 << 5,
    CHILIZ              = 1 << 6,
    STELLAR             = 1 << 7,
    AVALANCHE           = 1 << 8,
    ARBITRUM_ONE        = 1 << 9,
    FANTOM              = 1 << 10,
    XDAI                = 1 << 11,
    TOMOCHAIN           = 1 << 12,
    HUOBI_TOKEN         = 1 << 13,
    IOTEX               = 1 << 14,
    OKEX_CHAIN          = 1 << 15,
    CELO                = 1 << 16,
    TRON                = 1 << 17,
    ZILLIQA             = 1 << 18,
    WAVES               = 1 << 19,
    BINANCECOIN         = 1 << 20,
    EOS                 = 1 << 21,
    NEO                 = 1 << 22,
    KLAY_TOKEN          = 1 << 23
  };

  std::string     m_id;
  std::string     m_name;
  float           m_current_price;
  float           m_market_cap;
  int             m_market_cap_rank;
  float           m_ath;
  float           m_ath_change_percentage;
  std::string     m_ath_date;
  float           m_atl;
  float           m_atl_change_percentage;
  std::string     m_atl_date;
  float           m_price_change_percentage_1h_in_currency;
  float           m_price_change_percentage_24h_in_currency;
  float           m_price_change_percentage_7d_in_currency;
  float           m_price_change_percentage_14d_in_currency;
  float           m_price_change_percentage_30d_in_currency;
  std::map<Platform, std::string>    m_platforms;

};

const char* ValueOrEmpty(const char* s);