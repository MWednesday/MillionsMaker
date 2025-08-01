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
  // Using swtich statement is not possible for strings (unless you hash the string).
  // While a lot of if-else statements with string comparisons is also not great since they are usually every time O(n).
  // But the compiler can optimize the if-else heavily and use branch prediction.
  // Using a cached unordered_map could be perfect here since it allows O(1) average time complexity for lookups.
  // However, before that it spends time hashing the string and perhaps also the compiler has less of a room to optimize things.
  // Testing shows that using unordered_map in this case has no big speed up. Perhaps it would shine if we had even more pairs.
  // Keep in mind that unordered_map also stays in the RAM memory. But for the sake of the example, using unordered_map here.

  static const std::unordered_map<std::string, Coin::Platform> platformMap = {
    {"ethereum", Coin::Platform::ETHEREUM},
    {"binance-smart-chain", Coin::Platform::BINANCE_SMART_CHAIN},
    {"polygon-pos", Coin::Platform::POLYGON_POS},
    {"harmony-shard-0", Coin::Platform::HARMONY_SHARD_0},
    {"solana", Coin::Platform::SOLANA},
    {"chiliz", Coin::Platform::CHILIZ},
    {"stellar", Coin::Platform::STELLAR},
    {"avalanche", Coin::Platform::AVALANCHE},
    {"arbitrum-one", Coin::Platform::ARBITRUM_ONE},
    {"fantom", Coin::Platform::FANTOM},
    {"xdai", Coin::Platform::XDAI},
    {"tomochain", Coin::Platform::TOMOCHAIN},
    {"huobi-token", Coin::Platform::HUOBI_TOKEN},
    {"iotex", Coin::Platform::IOTEX},
    {"okex-chain", Coin::Platform::OKEX_CHAIN},
    {"celo", Coin::Platform::CELO},
    {"tron", Coin::Platform::TRON},
    {"zilliqa", Coin::Platform::ZILLIQA},
    {"waves", Coin::Platform::WAVES},
    {"binancecoin", Coin::Platform::BINANCECOIN},
    {"eos", Coin::Platform::EOS},
    {"neo", Coin::Platform::NEO},
    {"klay-token", Coin::Platform::KLAY_TOKEN},
    {"base", Coin::Platform::BASE},
    {"cronos", Coin::Platform::CRONOS}
  };

  auto it = platformMap.find(platformString);
  if(it != platformMap.end())
  {
    return it->second;
  }
  else
  {
    ReportDebug("Platform was not planned: %s", platformString.c_str()); // For debugging in case we want to add some missing platforms. Probably only worth adding if it has >10 occurences 
    return Coin::Platform::UNKNOWN;
  }
}

constexpr const char* ValueOrEmpty(const char* s)
{
    return s == nullptr ? "" : s;
}

bool CoinList::DeserializePlatform(const rapidjson::Value& arrayFile)
{
  if (arrayFile.IsArray())
  {
    std::string    platformName;
    std::string    platformAddress;
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