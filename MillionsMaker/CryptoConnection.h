#pragma once

#include "CoinList.h"
#include <vector>

class CryptoConnection
{
private:
  CoinList m_coinList;

  bool FillCoinList();

  bool m_SetupFinished = false;

  // ugly data needed for ImGui...:
  size_t m_coinsToParse = 0;
public:
  void SyncCryptoDataFromCoinGecko();
  void ScanAndReportSuccessfulCoins();

  const CoinList& GetCoinList() const { return m_coinList; }

  const size_t GetCoinsToParse() const { return m_coinsToParse; }
  void GetCoinInfoAndDeserialize(std::vector<int> pageNumbers);
  std::wstring GetBuyLink(const std::string& coinID);

  const bool IsSetupFinished() const { return m_SetupFinished; }
};