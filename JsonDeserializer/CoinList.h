#pragma once

#include "JSONBase.h"
#include "Coin.h"
#include <vector>
#include <mutex>

class CoinList : public JSONBase
{
public:
  virtual bool Deserialize(const rapidjson::Value& obj);
  using JSONBase::Deserialize;
  virtual bool DeserializePlatform(const rapidjson::Value& arrayFile);

  void SortCoinList();
  const std::vector<Coin>& GetCoinList() const { return m_CoinList;  }

private:
  std::vector<Coin> m_CoinList;
  std::mutex m_lock;

  Coin::Platform DeterminePlatform(const std::string& platformString);
};
