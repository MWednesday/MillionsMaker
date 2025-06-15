#pragma once

#include "CoinList.h"
#include <vector>

class CryptoConnection
{
private:
  CoinList m_CoinList;

  bool FillCoinList();

  bool m_SetupFinished = false;

  // ugly data needed for ImGui...:
  size_t m_CoinsToParse = 0;
public:
  void SyncCryptoDataFromCoinGecko();
  void ScanAndReportSuccessfulCoins();

  const CoinList& GetCoinList() const { return m_CoinList; }
  size_t GetCoinsToParse() const { return m_CoinsToParse; }
  void GetCoinInfoAndDeserialize(std::vector<int> pageNumbers);
  std::wstring GetBuyLink(const std::string& coinID);

  bool IsSetupFinished() const { return m_SetupFinished; }
};