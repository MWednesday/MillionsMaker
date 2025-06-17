#include "CryptoConnection.h"
#include "gecko.h"
#include "Logging.h"
#include "JSONBase.h"
#include <codecvt> // std::wstring_convert
#include "ScopeTimer.h"
#include "ThreadName.h"

bool IsError(const gecko::web::response& response)
{
  // https://support.coingecko.com/hc/en-us/articles/6472757474457-How-can-I-differentiate-between-the-status-codes-I-am-receiving-and-what-do-they-mean

  if (response.response_code == "200") // OK
  {
    return false;
  }

  return true;
}

void CryptoConnection::GetCoinInfoAndDeserialize(std::vector<int> pageNumbers)
{
  //ReportAssert(pageNumbers.size() > 0, "Ran function with 0 pages to process");

  gecko::coinsFunctions coins;
  gecko::web::response coinMarketDataResponse;
  int lastReportedPage = -1;

  for (int i = 0; i < pageNumbers.size();)
  {
    coinMarketDataResponse = coins.getMarkets("usd", 0, "1h,24h,7d,14d,30d", 0, pageNumbers[i], 250, false, "id_asc");

    if (IsError(coinMarketDataResponse))
    {
      if (pageNumbers[i] != lastReportedPage)
      {
        std::string& error = coinMarketDataResponse.text;
        while (!error.empty() && (error.back() == '\n')) { error.pop_back(); } // trim new lines at the end
        ReportError("Web request failed for page %u! Will retry soon. Reason: %s", pageNumbers[i], error.c_str());
        lastReportedPage = pageNumbers[i];
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(4000));
      continue;
    }

    m_CoinList.Deserialize(coinMarketDataResponse.text); // deserialize all coins in page
    ReportDebug("Processed all coins in page %u", pageNumbers[i]);
    i++;
  }
}

bool CryptoConnection::FillCoinList()
{
  MeasureScopeTime(ProcessingAllCoinsFromCoinGecko);
  gecko::api coinGecko;

  while (!coinGecko.ping())
  {
    ReportError("CoinGecko is offline! Waiting...");
    std::this_thread::sleep_for(std::chrono::milliseconds(4000));
  }

  gecko::coinsFunctions coins;
  gecko::web::response coinListResponse;
  gecko::web::response coinMarketDataResponse;

  // First, let's find out how many coins CoinGecko has and then we know how many pages we need
  coinListResponse = coins.listCoins(true);

  rapidjson::Document doc;
  if (!JSONBase::InitDocument(coinListResponse.text, doc))
  {
    ReportError("Failed to initialize document that we got from the server!");
    return false;
  }

  m_CoinsToParse = doc.GetArray().Size(); // Modify this to a number like 500 for faster launch during development
  const int numOfPages = ceil(m_CoinsToParse / 250.0);
  
  const int numOfThreads = std::thread::hardware_concurrency() - 3; // Leave some threads for other work
  std::vector<std::thread> workThreads;
  workThreads.reserve(numOfThreads);

  const int pagesPerThread = ceil((float)numOfPages / numOfThreads);

  int pageNum = 1;
  std::vector<int> pageNumbers;
  char threadName[22] = "Crypto Downloader ";
  for (int threadNum = 0; threadNum < numOfThreads - 1; threadNum++)
  {
    for (int k = 0; k < pagesPerThread && pageNum <= numOfPages; k++, pageNum++)
    {
      pageNumbers.push_back(pageNum);
    }

    if (!pageNumbers.empty())
    {
      std::thread& thread = workThreads.emplace_back(&CryptoConnection::GetCoinInfoAndDeserialize, this, std::move(pageNumbers));
      std::snprintf(threadName + 18, sizeof(threadName) - 18, "%d", threadNum);
      SetThreadName(thread, threadName);
    }
  }

  for (int threadNum = 0; threadNum < workThreads.size(); threadNum++)
  {
    if (workThreads[threadNum].joinable())
    {
      workThreads[threadNum].join();
    }
  }

  {
    MeasureScopeTime(SortCoinList);
    m_CoinList.SortCoinList();
  }

  {
    MeasureScopeTime(DeserializePlatform);
    m_CoinList.DeserializePlatform(doc);
  }

  m_SetupFinished = true;
  return true;
}

void CryptoConnection::SyncCryptoDataFromCoinGecko()
{
  std::string allCoins;
  m_CoinList.Deserialize(allCoins);
  FillCoinList();
  ReportInfo("\nCoin list size = %d\n\n", m_CoinList.GetCoinList().size());
}

void CryptoConnection::ScanAndReportSuccessfulCoins()
{
  for (const Coin& coin : m_CoinList.GetCoinList())
  {
    if (coin.m_price_change_percentage_1h_in_currency > 10 &&
        coin.m_price_change_percentage_24h_in_currency > 20 &&
        coin.m_price_change_percentage_7d_in_currency > 100 &&
        coin.m_price_change_percentage_14d_in_currency > 100)
    {
      ReportInfo("%s grew: 1h = %f%%,   24h = %f%%,  7d = %f%%,   14d = %f%%", coin.m_name.c_str(), coin.m_price_change_percentage_1h_in_currency, coin.m_price_change_percentage_24h_in_currency, coin.m_price_change_percentage_7d_in_currency, coin.m_price_change_percentage_14d_in_currency);
    }
  }
}

std::wstring CryptoConnection::GetBuyLink(const std::string& coinID)
{
  // Could also use the following method - instead of doing any CoinGecko calls, just find cached info in coin's platform. The issue is that we won't be able to open coin's official link
//auto const coin = std::find(GetCoinList().GetCoinList().begin(), GetCoinList().GetCoinList().end(), coinID);

//if (coin == GetCoinList().GetCoinList().end())
//{
//  ReportError("Failed to find the coin %s during GetBuyLink()", coinID.c_str());
//  return L"";
//}
//if (coin->m_platforms.empty())
//{
//  ReportError("No info on which platforms %s can be found.", coinID.c_str());
//}

//// the first element should be the main platform
//const Coin::Platform platform = coin->m_platforms.begin()->first;
//const std::string address = coin->m_platforms.begin()->second;

  gecko::api coinGecko;
  if (!coinGecko.ping())
  {
    ReportError("CoinGecko is offline! Failed to get coin's platform.");
    return L"";
  }

  gecko::coinsFunctions coins;
  gecko::web::response response;
  response = coins.getCoinData(coinID, false, false, false, false, false, false);

  rapidjson::Document doc;
  if (!JSONBase::InitDocument(response.text, doc))
  {
    ReportError("Failed to open the document in order to get coin's platform.");
    return L"";
  }

  const rapidjson::Value& platforms = doc["platforms"];
  std::string platform;
  std::string address;
  if (platforms.MemberCount() > 0)
  {
    // the first element should be the main platform
    platform = platforms.MemberBegin()->name.GetString();
    address = platforms.MemberBegin()->value.GetString();
  }

  std::string buy_link;
  if (platform == "ethereum" && address != "")
  {
    buy_link = "https://app.uniswap.org/#/swap?outputCurrency=" + address;
  }
  else if (platform == "binance-smart-chain" && address != "")
  {
    buy_link = "https://pancakeswap.finance/swap?outputCurrency=" + address + "&inputCurrency=0x55d398326f99059ff775485246999027b3197955";
  }
  else if (platform == "" || address == "") // coin probably has a custom platform
  {
    ReportError("No info on which platforms %s can be found. Will open its homepage instead.", coinID.c_str());
    const rapidjson::Value& links = doc["links"];
    buy_link = links["homepage"].GetArray()[0].GetString();
  }
  else
  {
    ReportError("Coin %s has a not very popular blockchain platform %s which hasn't been set up yet in MillionsMaker. Will open CoinGecko instead.", coinID.c_str(), platform.c_str());
    buy_link = "https://www.coingecko.com/en/coins/" + coinID + "#markets";
  }

  //string to wstring
  // Get size of the wstring. It can be different from std::string due to possible special characters and each char is at least 2 bytes
  int wideCharLen = MultiByteToWideChar(CP_UTF8, 0, buy_link.c_str(), -1, nullptr, 0);
  std::wstring wstr(wideCharLen, L'\0');
  MultiByteToWideChar(CP_UTF8, 0, buy_link.c_str(), -1, &wstr[0], wideCharLen);
  return wstr;
}