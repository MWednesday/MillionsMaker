#pragma once

#include "JSONBase.h"
#include <vector>
#include <set>
#include <mutex>
#include "Coin.h"

class CoinList : public JSONBase
{
public:
  //CoinList();
  //virtual ~CoinList();
  virtual bool Deserialize(const rapidjson::Value& obj);
  using JSONBase::Deserialize;
  virtual bool DeserializePlatform(const rapidjson::Value& arrayFile);

  virtual void PrintInfo();

  const std::vector<Coin>& GetCoinList() const { return m_CoinList;  }
  const std::set<Coin>& GetCoinList2() const { return m_CoinList2; }

private:
  std::vector<Coin> m_CoinList;
  std::set<Coin> m_CoinList2;
  std::mutex m_lock;

  Coin::Platform DeterminePlatform(std::string platformString);
};
