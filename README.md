# Cryptocurrency tracker-helper

Click for demo video:

[![Demo video](https://img.youtube.com/vi/AKAzLV8aGFE/0.jpg)](https://www.youtube.com/watch?v=AKAzLV8aGFE)

This application is written entirely in C++.
It gets over 17,000 cryptocurrency info from CoinGecko. Then it allows to sort, search, quickly open CoinGecko to look at graphs, and quickly open buy link on a platform which the coin supports.

It uses IMGUI for GUI, uses CoinGecko API and many work threads to quickly download and deserialize cryptocurrencies' info.

## Setup
After you build the MillionsMaker, you might need to copy dependency dlls into the exe folder.

Also, you will need to fill config.ini with your CoinGecko API key.
