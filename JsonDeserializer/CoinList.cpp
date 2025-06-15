#include "CoinList.h"
#include "ScopeTimer.h"
#include <algorithm> // find_if

bool CoinList::Deserialize(const rapidjson::Value & arrayFile)
{
  if (arrayFile.IsArray())
  {
#ifdef _DEBUG
    std::string info;
#endif
    for (const rapidjson::Value & coinData : arrayFile.GetArray())
    {
      Coin coin;
      coin.Deserialize(coinData);
      m_lock.lock();
      m_CoinList.push_back(coin);
      m_lock.unlock();
#ifdef _DEBUG
      info = "Added " + coin.m_name + " to the coin list.";
      ReportDebug(info.c_str()); // Passing preformatted string to avoid ??? in some coin names. TODO: Check the reasons
#endif
    }
  }
  else
  {
    ReportError("Error: file is not an array!");
  }
  return true;
}

Coin::Platform CoinList::DeterminePlatform(const std::string& platformString)
{
  if (platformString == "ethereum") // TODO: could check performance of if-else vs switch statement
  {
    return Coin::Platform::ETHEREUM;
  }
  else if (platformString == "binance-smart-chain")
  {
    return Coin::Platform::BINANCE_SMART_CHAIN;
  }
  else if (platformString == "polygon-pos")
  {
    return Coin::Platform::POLYGON_POS;
  }
  else if (platformString == "harmony-shard-0")
  {
    return Coin::Platform::HARMONY_SHARD_0;
  }
  else if (platformString == "solana")
  {
    return Coin::Platform::SOLANA;
  }
  else if (platformString == "chiliz")
  {
    return Coin::Platform::CHILIZ;
  }
  else if (platformString == "stellar")
  {
    return Coin::Platform::STELLAR;
  }
  else if (platformString == "avalanche")
  {
    return Coin::Platform::AVALANCHE;
  }
  else if (platformString == "arbitrum-one")
  {
    return Coin::Platform::ARBITRUM_ONE;
  }
  else if (platformString == "fantom")
  {
    return Coin::Platform::FANTOM;
  }
  else if (platformString == "xdai")
  {
    return Coin::Platform::XDAI;
  }
  else if (platformString == "tomochain")
  {
    return Coin::Platform::TOMOCHAIN;
  }
  else if (platformString == "huobi-token")
  {
    return Coin::Platform::HUOBI_TOKEN;
  }
  else if (platformString == "iotex")
  {
    return Coin::Platform::IOTEX;
  }
  else if (platformString == "okex-chain")
  {
    return Coin::Platform::OKEX_CHAIN;
  }
  else if (platformString == "celo")
  {
    return Coin::Platform::CELO;
  }
  else if (platformString == "tron")
  {
    return Coin::Platform::TRON;
  }
  else if (platformString == "zilliqa")
  {
    return Coin::Platform::ZILLIQA;
  }
  else if (platformString == "waves")
  {
    return Coin::Platform::WAVES;
  }
  else if (platformString == "binancecoin")
  {
    return Coin::Platform::BINANCECOIN;
  }
  else if (platformString == "eos")
  {
    return Coin::Platform::EOS;
  }
  else if (platformString == "neo")
  {
    return Coin::Platform::NEO;
  }
  else if (platformString == "klay-token")
  {
    return Coin::Platform::KLAY_TOKEN;
  }
  else if (platformString == "base")
  {
    return Coin::Platform::BASE;
  }
  else if (platformString == "cronos")
  {
    return Coin::Platform::CRONOS;
  }
  else
  {
    ReportDebug("Platform was not planned: %s", platformString.c_str()); // For debugging in case we want to add some missing platforms. Probably only worth adding if it has >10 occurences 
    return Coin::Platform::UNKNOWN;
  }
}

//Coin* find(std::string valueToFind, std::vector<Coin>& array)
//{
//  int pos = 0;
//  int length = array.size();
//  int limit = std::min(length, 1);
//  while (limit < length && array[limit] < valueToFind)
//  {
//    pos = limit + 1;
//    limit = std::min(length, limit * 2 + 1);
//  }
//  while (pos < limit)
//  {
//    int testpos = pos + ((limit - pos) >> 1);
//
//    if (array[testpos] < valueToFind)
//      pos = testpos + 1;
//    else
//      limit = testpos;
//  }
//  return (pos < length&& array[pos] == valueToFind ? pos : -1);
//}

bool CoinList::DeserializePlatform(const rapidjson::Value& arrayFile)
{
  if (arrayFile.IsArray())
  {
    std::string platformName;
    std::string platformAddress;
    Coin::Platform platformToInsert = Coin::Platform::UNSET;
    std::string coinID;

    for (const rapidjson::Value& coinData : arrayFile.GetArray())
    {
      auto platformList = coinData["platforms"].GetObject();

      for (const auto& platform : platformList)
      {
        platformName = ValueOrEmpty(platform.name.GetString());
        platformAddress = ValueOrEmpty(platform.value.GetString());
        platformToInsert = DeterminePlatform(platformName);

        // find to which coin does this data belong and add it
        coinID = coinData["id"].GetString();

        auto it = std::lower_bound(m_CoinList.begin(), m_CoinList.end(), coinID); // this requires a sorted list

        if (it != m_CoinList.end())
        {
          it->m_platforms.emplace( platformToInsert, std::move(platformAddress) );
        }
        else
        {
          ReportError("Failed to add platforms for coin %s", coinID.c_str());
        }
      }
    }
  }
  else
  {
    ReportError("Error: file is not an array!");
    return false;
  }
  return true;
}

void CoinList::SortCoinList()
{
  std::sort(m_CoinList.begin(), m_CoinList.end());
}