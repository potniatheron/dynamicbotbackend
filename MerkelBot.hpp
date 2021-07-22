#pragma once

#include "OrderBook.hpp"
#include "OrderBookEntry.hpp"
#include "Wallet.hpp"
#include <fstream>
#include <vector>

class MerkelBot {
public:
  MerkelBot();
  /** Call this to start the bot */
  void init(OrderBook orderBook, Wallet wallet, int input);

private:
  void calculateEMA(OrderBook orderBook, Wallet wallet, OrderBookEntry entry);
  void placeOrder(OrderBook orderBook, Wallet wallet, OrderBookType type,
                OrderBookEntry entry);
  void calculateMA(OrderBookEntry entry);
  std::string getAction(OrderBookType type);
  OrderBookEntry buildObe(OrderBookType type, OrderBookEntry entry);
  float getSuitableAmount(OrderBookEntry entry);
  std::tuple<int, int> getDeltaThresholds(OrderBookEntry entry);

  std::string currentTime;
  std::vector<double> movingAverages;
  double movingAverageAcc = 0;
  double bidEntriesCounter = 0;

  // The Timestamp counter will be used to keep track of the time "passing" within the orderBook.
  // The bot will take a snapshot (i.e. will compute the Exponential Moving Average) every 10 snapshots
  double timestampCounter = 0;
  // The Snapshot counter will be used within the Exponential Moving Average formula to compute its value
  double snapshotCounter = 0;
  std::string currentTimestamp = "";
  std::ofstream logger;
};
