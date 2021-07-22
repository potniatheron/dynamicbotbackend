#pragma once

#include "OrderBook.hpp"
#include "MerkelBot.hpp"
#include "OrderBookEntry.hpp"
#include "Wallet.hpp"
#include <vector>

class MerkelMain {
public:
  MerkelMain();
  /** Call this to start the sim */
  void init();

private:
  void printMenu();
  void printBotSubmenu();
  void startBot();
  void printHelp();
  void printMarketStats();
  void enterAsk();
  void enterBid();
  void printWallet();
  void gotoNextTimeframe();
  int getUserOption();
  int getUserBotSubmenuOption();
  void processUserOption(int userOption);

  std::string currentTime;

//  OrderBook orderBook{"20200317.csv"};
  OrderBook orderBook{"20200601.csv"};
    
  Wallet wallet;
  MerkelBot merkelBot;
};
