#include "MerkelBot.hpp"
#include "OrderBookEntry.hpp"
#include <iostream>
#include <tuple>
#include <vector>

MerkelBot::MerkelBot() {}
// Initializing bot. The function receives the MerkelMain orderBook, wallet and selected product input
void MerkelBot::init(OrderBook orderBook, Wallet wallet, int input) {
  // automatedProduct translates the user's numeric input to a string indicating the selected product that the bot was asked to trade
  std::string automatedProduct;

  // User input 1 corresponds to BTC/USDT
  if (input == 1) {
    automatedProduct = "BTC/USDT";
  }
  // User input 2 corresponds to ETH/BTC
  if (input == 2) {
    automatedProduct = "ETH/BTC";
  }
  // User input 2 corresponds to DOGE/BTC
  if (input == 3) {
    automatedProduct = "DOGE/BTC";
  }
  // Logger is an output stream where we will be logging all of the bot's operations
  logger.open("output.txt");
  // First line of the logging document, indicating which product the bot is about to trade
  logger << "MerkelBot | Trading App | Automating " << automatedProduct
         << " trades \n"
         << std::endl;

  // In order to calculate the Exponential Moving Average, the average bid values are considered
  // We pass the selected orderBookType (bid) and the name of the product that was chosen by the user
  // to the relevant function
  std::vector<OrderBookEntry> bidEntries =
      orderBook.getOrdersByTypeAndProduct(OrderBookType::bid, automatedProduct);

  // We cycle over all of the orderbBook entries of type bid, as they are retrieved by communicating with the orderBook
  for (OrderBookEntry &entry : bidEntries) {
    // We immediately increase the bidEntries counter. Such value will later be useful to calculate the first Moving Average
    bidEntriesCounter++;

    // Three logical conditions are defined in order to decide the bot's control flow
    // Such booleans have been extracted to improve readability of the following if else clauses
    // If the timestamp we are iterating on is a new timestamp i.e. different from the previous one, this will be true
    bool isNewTimestamp = entry.timestamp != currentTimestamp;
    // If this is the first moving average we calculate, this will be true
    bool isFirstAverage = movingAverages.size() == 0;
    // If we have been seeing 10 different timestamps up to this moment, this will be true
    bool isNewSnapshotTime = timestampCounter == 10;

    if (isFirstAverage) {
      // Since the formula for Exponential Moving Average is recursive, we have to start from a regular Moving Average
      // A Moving Average is less prices than an Exponential Moving Average but will let us get started with the calculations chain
      movingAverageAcc = movingAverageAcc + entry.price;
    }
    if (isNewTimestamp) {
      // Take not of current timestamp
      currentTimestamp = entry.timestamp;
    }
    // If it's not time to take a new snapshot and we are seeing a new timestamp, we will enter in this flow
    if (!isNewSnapshotTime && isNewTimestamp) {
      // Increment timestamp counter
      timestampCounter++;
    }

    // If we are seeing a new timestamp and it's time for the bot to take a new snapshot (i.e. calculate a new average), we will enter in this flow
    if (isNewTimestamp && isNewSnapshotTime) {
      // We reset the timestamp counter, because we will be counting once again from 0 to 10 the new timestamps that we will encounter after the current one
      timestampCounter = 0;
      // Increment snapshot counter
      snapshotCounter++;
      // If it's the first average we calculate (i.e. we don't have any former Exponential Moving Average value in our vector), we will enter in this flow
      if (isFirstAverage) {
        // call the Moving Average calculation function by passing it our current entry
        calculateMA(entry);
      } else {
        // call the Moving Average calculation function by passing it our current entry, plus the orderBook and the wallet to start making offers where suitable
        calculateEMA(orderBook, wallet, entry);
      }
    }
  }
    
  // After we finish iterating on all bids from the orderBook, the bot has run its course and we can close the "output.txt" filestream
  logger.close();
}

// Each of the three products (BTC, ETH, DOGE) has a different suitable amount that needs to be sold or bought, based on their value
float MerkelBot::getSuitableAmount(OrderBookEntry entry) {
  // If the bot is handling BTC, the deal size is 1
  if (entry.product == "BTC/USDT") {
    return 1;
  }
  // If the bot is handling ETH, the deal size is 10
  if (entry.product == "ETH/BTC") {
    return 10;
  }
  // If the bot is handling DOGE, the deal size is 100
  if (entry.product == "DOGE/BTC") {
    return 100;
  }
  // We return 0 if any other input is received by mistake
  return 0;
}

// Each of the three products (BTC, ETH, DOGE) has a different threshold that is given to the bot in order to make a deal
// Delta is the difference between the previous EMA and the current EMA. If we see that the EMA is starting to decrease/increase, we take the appropriate course of action
std::tuple<int, int> MerkelBot::getDeltaThresholds(OrderBookEntry entry) {
  // If the bot is handling BTC, the threshold values for triggering any bot action is 0.2, -0.2
  if (entry.product == "BTC/USDT") {
    return std::make_tuple(0.2, -0.2);
  }
  // If the bot is handling ETH, the threshold values for triggering any bot action is 8.44151e-06, -5.0012e-06
  if (entry.product == "ETH/BTC") {
    return std::make_tuple(8.44151e-06, -5.0012e-06);
  }
  // If the bot is handling DOGE, the threshold values for triggering any bot action is 0.001, -0.001
  if (entry.product == "DOGE/BTC") {
    return std::make_tuple(0.001, -0.001);
  }
  // We return a 0,0 tuple if any other input is received by mistake
  return std::make_tuple(0, 0);
}

// The function calculates the Exponential Moving Average and based on its value evaluates the course of action for the bot flow
void MerkelBot::calculateEMA(OrderBook orderBook, Wallet wallet,
                             OrderBookEntry entry) {
  // The previous EMA value is picked from the movingAverages vector
  // Note that the movingAverages vector is filled up in this very function, as well as in the calculateMA function (for the first element)
  float oldEMA = movingAverages[movingAverages.size() - 1];
  // We are referencing the following EMA formula:
    
  // For the sake of readability and to better control the code, we divide the formula in two parts, that we will later sum
  // .0 is added in order to handle the decimal part
  // The price of the entry we are currently iterating on is multiplied by the smoothing factor (2) divided by the total number of movingAverages calculated up to this moment plus 1
  float newEMA_1 = entry.price * (2.0 / (snapshotCounter + 1.0));
  // The previous EMA is multiplied by the smoothing factor (2) divided by the total number of movingAverages calculated up to this moment plus 1
  float newEMA_2 = oldEMA * (1.0 - (2.0 / (snapshotCounter + 1.0)));

  // In order to avoid delta calculation errors, we set the EMA to the arbitrary value of 1 if it happens to be 0
  // Note that this only happens with very small numbers in the beginning of the EMA calculations
  if (newEMA_1 == 0) {
    newEMA_1 = 1;
  }
  if (newEMA_2 == 0) {
    newEMA_2 = 1;
  }
  // The EMA formula is completed by summing the two halves
  float newEMA = newEMA_1 + newEMA_2;
  // The difference between the previous EMA and the current EMA is calculated
  // At crossover, i.e. change in direction, we perform a sell/buy action based on this value and the selected thresholds
  float delta = oldEMA - newEMA;
  // Based on the user-selected crypto, the bot will get the relevant thresholds from the deltaThresholds tuple
  float acceptedBidDelta = std::get<0>(getDeltaThresholds(entry));
  float acceptedAskDelta = std::get<1>(getDeltaThresholds(entry));

  // We print the current bot situation on the logging file
  logger << "calculateEMA "
         << "Old EMA is" << oldEMA << " | New EMA is " << newEMA
         << " | Δ with previous EMA is " << delta << std::endl;

  // If there exists the conditions to place a bid, bot will enter this flow
  if (delta > acceptedBidDelta) {
    // Call the order function and pass it all relevant values
    placeOrder(orderBook, wallet, OrderBookType::bid, entry);
  }
    // If current delta corresponds to no bid/ask conditions, bot will enter this flow
  if (delta > acceptedAskDelta && delta < acceptedBidDelta) {
    // We print the current bot situation on the logging file
    logger << "Δ with previous EMA is irrelevant. Bot is sleeping..."
           << std::endl;
  }
  // If there exists the conditions to place an ask, bot will enter this flow
  if (delta < acceptedAskDelta) {
    // Call the order function and pass it all relevant values
      placeOrder(orderBook, wallet, OrderBookType::ask, entry);
  }

  // We push the newly generated EMA to our EMA vector
  movingAverages.push_back(newEMA);
}

// This function will be called when creating our first snapshot
void MerkelBot::calculateMA(OrderBookEntry entry) {
  // We print the current bot situation on the logging file
  logger << "calculateMA " << movingAverageAcc / bidEntriesCounter << std::endl;
  // We push the newly generated MA to our EMA vector. This first element will be less precise than an actual EMA but will let us start the calculations from an approximated point
  // Our MA is calculated by dividing the total amount that we have been accumulating with the previous iterations, and dividing it by the total number of bids taken into account
  movingAverages.push_back(movingAverageAcc / bidEntriesCounter);
}

// Based on the Orderbooktype, we determine the name of the action for clarity and logging purposes
std::string MerkelBot::getAction(OrderBookType type) {
  // If ordertype is ask, this translates to a SELL action
  if (type == OrderBookType::ask) {
    return "SELL";
  }
  // If ordertype is bid, this translates to a BUY action
  if (type == OrderBookType::bid) {
    return "BUY";
  }
  return "Invalid action type";
}

// Based on the bot data and the bot assumptions we build the Order Book Entry that will be used to place an order
OrderBookEntry MerkelBot::buildObe(OrderBookType type, OrderBookEntry entry) {
  // The total amount of cryptocurrency that we buy is defined a priori in the relevant function
  float amount = getSuitableAmount(entry);
  // For exemplification purposes only, we define our entry price in order to make sure that we are awarded the best offer
  // This will not make our bot well performing, but will let us complete our proof of concept
  double obePrice = 0;
  // Make ask price lower, to maximize winning chances
  if (type == OrderBookType::ask) {
    obePrice = entry.price * 0.9;
  }
  // Make bid price higher, to maximize winning chances
  if (type == OrderBookType::bid) {
    obePrice = entry.price * 1.1;
  }
  // Create a new OrderBookEntry entity with all of the appropriate values
  OrderBookEntry obe{obePrice, amount, currentTimestamp, entry.product, type};
  // Overwrite the existing username by setting it to "bot"
  obe.username = "bot";
  // Return the OBE to the calling function for finallt placing it
  return obe;
}

// This function will interact with the Orderbook and insert an order. Then it will trigger the matching method in order to simulate the exchange behaviour
void MerkelBot::placeOrder(OrderBook orderBook, Wallet wallet, OrderBookType type,
                         OrderBookEntry entry) {
  // We print the current bot situation on the logging file
  logger << "Placing a " << getAction(type) << " order" << std::endl;
  // Leaving a console log in order to keep track of what's happening on the terminal side as well
  std::cout << "Placing a " << getAction(type) << " order" << std::endl;

  // Call the assembling function that generates our obe
  OrderBookEntry obe = buildObe(type, entry);
  // Pass the generated obe to the orderBook and make it ready for processing
  orderBook.insertOrder(obe);

  // Set a threshould for acceptance of sliced amounts, if the bid/ask is competing with others of the same value
  float acceptedAmount = obe.amount / 3;

  // Call matching simulator and get a list of the accepted bot sales
  std::vector<OrderBookEntry> sales =
      orderBook.matchAsksToBids(entry.product, currentTimestamp);
  // Iterate on the sales, and retrieve the orderBook logging data
  for (OrderBookEntry &sale : sales) {
    // Split the product couple for logging purposes
    std::vector<std::string> tokens = CSVReader::tokenise(sale.product, '/');
    // We print the current bot situation on the logging file
    logger << getAction(type) << " offer was accepted." << std::endl;
    logger << sale.controlString << std::endl;
    logger << "Processing " << sale.amount << " " << tokens[0] << " for "
           << sale.price << " " << tokens[1] << std::endl;

    // Now we get the accepted sale and check if the acceptedAmount hits our threshold. If so, we can finally open the wallet and complete the order
    if (sale.amount > acceptedAmount) {
      // We print the current bot situation on the logging file
      logger << "Minimum amount accepted for trade was hit." << std::endl;
      // We check that the wallet has enough currency to fulfill the accepted order
      if (wallet.canFulfillOrder(obe)) {
        // We print the current bot situation on the logging file
        logger << "Wallet has sufficient funds to proceed." << std::endl;
        logger << "Processing sale..." << std::endl;
        // We process the sale buy changing the wallet amounts
        wallet.processSale(sale);
      } else {
        // We print the current bot situation on the logging file
        logger << "Wallet has insufficient funds. " << std::endl;
      }
    } else {
      // If the acceptedAmount didn't hit our minimal threshold, we withdraw our order with the relevant, newly built, function
      orderBook.removeOrder(obe);
    }
  }

  // We print the new wallet situation after completing the transaction on the logging file
  logger << "New wallet situation" << std::endl;
  logger << wallet.toString() << std::endl;
}
