#include "CryptoConnection.h"
#include "gecko.h"
#include "Logging.h"
#include "JSONBase.h"
#include <codecvt> // std::wstring_convert
#include "ScopeTimer.h"

#pragma optimize("", off)

void CryptoConnection::GetCoinInfoAndDeserialize(std::vector<int> pageNumbers)
{
  gecko::coinsFunctions coins;
  gecko::web::response coinMarketDataResponse;
  for (int pageNumber : pageNumbers)
  {
    coinMarketDataResponse = coins.getMarkets("usd", 0, "1h,24h,7d,14d,30d", 0, pageNumber, 250, false, "id_asc");
    m_coinList.Deserialize(coinMarketDataResponse.text); // deserialize all coins in page
  }
}

bool CryptoConnection::FillCoinList()
{
  MeasureScopeTime(ProcessingAllCoinsFromCoinGecko);
  gecko::api coinGecko;
  if (!coinGecko.ping())
  {
    ReportError("CoinGecko offline!");
    return false;
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

  m_coinsToParse = doc.GetArray().Size(); // Modify this to a number like 500 for faster launch during development
  const int numOfPages = ceil(m_coinsToParse / 250.0);
  
  const int NUMOFTHREADS = 20;
  std::thread workThreads[NUMOFTHREADS]; // I guess since this is a regular array it doesn't use heap, but the issue is that it will auto-create default(empty) threads which we don't need and they later get replaced by real threads
  int workForNineThreads = ceil((float)numOfPages / NUMOFTHREADS);
  int workForTenth = numOfPages % workForNineThreads;

  int pageNum = 1;
  std::vector<int> pageNumbers;
  for (int threadNum = 0; threadNum < NUMOFTHREADS - 1; threadNum++)
  {
    for (int k = 0; k < workForNineThreads && pageNum <= numOfPages; k++, pageNum++)
    {
      pageNumbers.push_back(pageNum);
    }
    workThreads[threadNum] = std::thread(&CryptoConnection::GetCoinInfoAndDeserialize, this, std::move(pageNumbers)); // we don't need elements here after this, so can move
  }

  for (; pageNum <= numOfPages; pageNum++)
  {
    pageNumbers.push_back(pageNum);
  }
  workThreads[NUMOFTHREADS - 1] = std::thread(&CryptoConnection::GetCoinInfoAndDeserialize, this, std::move(pageNumbers));

  for (int threadNum = 0; threadNum < NUMOFTHREADS; threadNum++)
  {
    workThreads[threadNum].join();
  }

  //for (int pageNum = 1; pageNum <= numOfPages; pageNum++)
  //{
  //  coinMarketDataResponse = coins.getMarkets("usd", 0, "1h,24h,7d,14d,30d", 0, pageNum, 250, false, "id_asc");
  //  m_coinList.Deserialize(coinMarketDataResponse.text); // deserialize all coins in page
  //}

  m_coinList.DeserializePlatform(doc);

  m_SetupFinished = true;
  return true;
}

void CryptoConnection::SyncCryptoDataFromCoinGecko()
{
  std::string allCoins;
  m_coinList.Deserialize(allCoins);
  FillCoinList();
  ReportInfo("\nCoin list size = %d\n\n", m_coinList.GetCoinList().size());
}

void CryptoConnection::ScanAndReportSuccessfulCoins()
{
  std::set<Coin> successfulCoins;
  for (Coin coin : m_coinList.GetCoinList())
  {
    if (coin.m_price_change_percentage_1h_in_currency > 10 &&
      coin.m_price_change_percentage_24h_in_currency > 20 &&
      coin.m_price_change_percentage_7d_in_currency > 100 &&
      coin.m_price_change_percentage_14d_in_currency > 100)
    {
      successfulCoins.insert(coin);
      ReportInfo("%s grew: 1h = %f%%,   24h = %f%%,  7d = %f%%,   14d = %f%%", coin.m_name.c_str(), coin.m_price_change_percentage_1h_in_currency, coin.m_price_change_percentage_24h_in_currency, coin.m_price_change_percentage_7d_in_currency, coin.m_price_change_percentage_14d_in_currency);
    }
  }
}

std::wstring CryptoConnection::GetBuyLink(const std::string& coinID)
{
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
  auto platformChar = platforms[0].GetString(); // the first element should be the main platform
  std::string platform = platformChar;
  std::string address = platforms[platformChar].GetString();

  std::string buy_link = "";
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
    const rapidjson::Value& links = doc["links"];
    buy_link = links["homepage"].GetArray()[0].GetString();
  }
  else
  {
    ReportError("Coin %s has a not very popular blockchain platform %s which hasn't been set up yet in MillionsMaker. Will open CoinGecko instead.", coinID, platform);
    buy_link = "https://www.coingecko.com/en/coins/" + coinID + "#markets";
  }

  //string to wstring
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
  return conv.from_bytes(buy_link);
}