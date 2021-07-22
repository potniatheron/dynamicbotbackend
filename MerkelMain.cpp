#include "CSVReader.hpp"
#include "MerkelMain.hpp"
#include "OrderBookEntry.hpp"
#include <iostream>
#include <vector>

MerkelMain::MerkelMain() {}

void MerkelMain::init() {
  int input;
  currentTime = orderBook.getEarliestTime();

  wallet.insertCurrency("BTC", 10);
  wallet.insertCurrency("USDT", 100000);
  wallet.insertCurrency("ETH", 50);
  wallet.insertCurrency("DOGE", 50000);

  while (true) {
    printMenu();
    input = getUserOption();
    processUserOption(input);
  }
}

void MerkelMain::printMenu() {
  // 1 print bot
  std::cout << "1: Start bot " << std::endl;
  // 1 print help
  std::cout << "2: Print help " << std::endl;
  // 2 print exchange stats
  std::cout << "3: Print exchange stats" << std::endl;
  // 3 make an offer
  std::cout << "4: Make an offer " << std::endl;
  // 4 make a bid
  std::cout << "5: Make a bid " << std::endl;
  // 5 print wallet
  std::cout << "6: Print wallet " << std::endl;
  // 6 continue
  std::cout << "7: Continue " << std::endl;

  std::cout << "============== " << std::endl;

  std::cout << "Current time is: " << currentTime << std::endl;
}

void MerkelMain::printBotSubmenu() {
  // 1 print btc/usdt
  std::cout << "1: Automate BTC/USDT trades " << std::endl;
  // 2 print eth/btc
  std::cout << "2: Automate ETH/BTC trades " << std::endl;
  // 3 print doge/btc
  std::cout << "3: Automate DOGE/BTC trades " << std::endl;

  std::cout << "Current time is: " << currentTime << std::endl;
}

void MerkelMain::startBot() {
  std::cout << "Select product to automate" << std::endl;
  int input;

  while (true) {
    printBotSubmenu();
    input = getUserOption();
    std::cout << "The Trading Bot is starting..." << std::endl;

    merkelBot.init(orderBook, wallet, input);
  }
}

void MerkelMain::printHelp() {
  std::cout << "Help - your aim is to make money. Analyse the market and make "
               "bids and offers. "
            << std::endl;
}

void MerkelMain::printMarketStats() {
  for (std::string const &p : orderBook.getKnownProducts()) {
    std::cout << "Product: " << p << std::endl;
    std::vector<OrderBookEntry> entries =
        orderBook.getOrders(OrderBookType::ask, p, currentTime);
    std::cout << "Asks seen: " << entries.size() << std::endl;
    std::cout << "Max ask: " << OrderBook::getHighPrice(entries) << std::endl;
    std::cout << "Min ask: " << OrderBook::getLowPrice(entries) << std::endl;
  }
  // std::cout << "OrderBook contains :  " << orders.size() << " entries" <<
  // std::endl; unsigned int bids = 0; unsigned int asks = 0; for
  // (OrderBookEntry& e : orders)
  // {
  //     if (e.orderType == OrderBookType::ask)
  //     {
  //         asks ++;
  //     }
  //     if (e.orderType == OrderBookType::bid)
  //     {
  //         bids ++;
  //     }
  // }
  // std::cout << "OrderBook asks:  " << asks << " bids:" << bids << std::endl;
}

void MerkelMain::enterAsk() {
  std::cout << "Make an ask - enter the amount: product,price, amount, eg  "
               "ETH/BTC,200,0.5"
            << std::endl;
  std::string input;
  std::getline(std::cin, input);

  std::vector<std::string> tokens = CSVReader::tokenise(input, ',');
  if (tokens.size() != 3) {
    std::cout << "MerkelMain::enterAsk Bad input! " << input << std::endl;
  } else {
    try {
      OrderBookEntry obe = CSVReader::stringsToOBE(
          tokens[1], tokens[2], currentTime, tokens[0], OrderBookType::ask);
      obe.username = "simuser";
      if (wallet.canFulfillOrder(obe)) {
        std::cout << "Wallet looks good. " << std::endl;
        orderBook.insertOrder(obe);
      } else {
        std::cout << "Wallet has insufficient funds . " << std::endl;
      }
    } catch (const std::exception &e) {
      std::cout << " MerkelMain::enterAsk Bad input " << std::endl;
    }
  }
}

void MerkelMain::enterBid() {
  std::cout << "Make an bid - enter the amount: product,price, amount, eg  "
               "ETH/BTC,200,0.5"
            << std::endl;
  std::string input;
  std::getline(std::cin, input);

  std::vector<std::string> tokens = CSVReader::tokenise(input, ',');
  if (tokens.size() != 3) {
    std::cout << "MerkelMain::enterBid Bad input! " << input << std::endl;
  } else {
    try {
      OrderBookEntry obe = CSVReader::stringsToOBE(
          tokens[1], tokens[2], currentTime, tokens[0], OrderBookType::bid);
      obe.username = "simuser";

      if (wallet.canFulfillOrder(obe)) {
        std::cout << "Wallet looks good. " << std::endl;
        orderBook.insertOrder(obe);
      } else {
        std::cout << "Wallet has insufficient funds . " << std::endl;
      }
    } catch (const std::exception &e) {
      std::cout << " MerkelMain::enterBid Bad input " << std::endl;
    }
  }
}

void MerkelMain::printWallet() { std::cout << wallet.toString() << std::endl; }

void MerkelMain::gotoNextTimeframe() {
  std::cout << "Going to next time frame. " << std::endl;
  for (std::string p : orderBook.getKnownProducts()) {
    std::cout << "matching " << p << std::endl;
    std::vector<OrderBookEntry> sales =
        orderBook.matchAsksToBids(p, currentTime);
    std::cout << "Sales: " << sales.size() << std::endl;
    for (OrderBookEntry &sale : sales) {
      std::cout << "Sale price: " << sale.price << " amount " << sale.amount
                << std::endl;
      if (sale.username == "simuser") {
        // update the wallet
        wallet.processSale(sale);
      }
    }
  }

  currentTime = orderBook.getNextTime(currentTime);
}

int MerkelMain::getUserOption() {
  int userOption = 0;
  std::string line;
  // *bug | This shows also after submenu selection
  std::cout << "Type in 1-7" << std::endl;
  std::getline(std::cin, line);
  try {
    userOption = std::stoi(line);
  } catch (const std::exception &e) {
    //
  }
  std::cout << "You chose: " << userOption << std::endl;
  return userOption;
}

int MerkelMain::getUserBotSubmenuOption() {
  int userOption = 0;
  std::string line;
  std::cout << "Type in 1-3" << std::endl;
  std::getline(std::cin, line);
  try {
    userOption = std::stoi(line);
  } catch (const std::exception &e) {
    //
  }
  std::cout << "You chose: " << userOption << std::endl;
  return userOption;
}

void MerkelMain::processUserOption(int userOption) {
  if (userOption == 1) {
    startBot();
  }
  if (userOption == 2) {
    printHelp();
  }
  if (userOption == 3) {
    printMarketStats();
  }
  if (userOption == 4) {
    enterAsk();
  }
  if (userOption == 5) {
    enterBid();
  }
  if (userOption == 6) {
    printWallet();
  }
  if (userOption == 7) {
    gotoNextTimeframe();
  }
}
