#include "OrderBook.hpp"
#include "CSVReader.hpp"
#include <algorithm>
#include <iostream>
#include <map>

/** construct, reading a csv data file */
OrderBook::OrderBook(std::string filename) {
  ordersMap = CSVReader::readCSVMap(filename);
}

/** return vector of all know products in the dataset*/
std::vector<std::string> OrderBook::getKnownProducts() {
  std::vector<std::string> products;
  // The vector has been replaced with a map data structure, and all of the relevant functions have been adapted as a consequence
  std::map<std::string, bool> prodMap;

  for (auto const &o : ordersMap) {
    std::vector<OrderBookEntry> obeList = o.second;
    for (OrderBookEntry &e : obeList) {
      prodMap[e.product] = true;
    }
  }

  // now flatten the map to a vector of strings
  for (auto const &e : prodMap) {
    products.push_back(e.first);
  }

  return products;
}

int OrderBook::getOrdersSize() { return ordersMap.size(); }

/** return vector of Orders according to the sent filters*/
std::vector<OrderBookEntry> OrderBook::getOrders(OrderBookType type,
                                                 std::string product,
                                                 std::string timestamp) {
  std::vector<OrderBookEntry> orders_sub;

  for (auto const &o : ordersMap) {
    if (o.first == timestamp) {
      std::vector<OrderBookEntry> obeList = o.second;
      for (OrderBookEntry &e : obeList) {
        if (e.orderType == type && e.product == product) {
          orders_sub.push_back(e);
        }
      }
    }
  }
  return orders_sub;
}

/** return vector of Orders according to type*/
std::vector<OrderBookEntry>
OrderBook::getOrdersByTypeAndProduct(OrderBookType type, std::string product) {
  std::vector<OrderBookEntry> orders_sub;
  for (auto const &o : ordersMap) {
    std::vector<OrderBookEntry> obeList = o.second;
    for (OrderBookEntry &e : obeList) {
      if (e.orderType == type && e.product == product) {
        orders_sub.push_back(e);
      }
    }
  }

  return orders_sub;
}

double OrderBook::getHighPrice(std::vector<OrderBookEntry> &orders) {
  double max = orders[0].price;
  for (OrderBookEntry &e : orders) {
    if (e.price > max)
      max = e.price;
  }
  return max;
}

double OrderBook::getLowPrice(std::vector<OrderBookEntry> &orders) {
  double min = orders[0].price;
  for (OrderBookEntry &e : orders) {
    if (e.price < min)
      min = e.price;
  }
  return min;
}

std::string OrderBook::getEarliestTime() { return ordersMap.begin()->first; }

std::string OrderBook::getNextTime(std::string timestamp) {
  std::string next_timestamp = "";
  for (auto const &e : ordersMap) {
    if (e.first > timestamp) {
      next_timestamp = timestamp;
      break;
    }
  }

  if (next_timestamp == "") {
    next_timestamp = orders[0].timestamp;
  }
  return next_timestamp;
}

// This function has been edited to reflect the speed optimizations
// It will now select the map element (which is a vector) by its timestamp, and then push the order in the vector
void OrderBook::insertOrder(OrderBookEntry &order) {
  ordersMap[order.timestamp].push_back(order);
}

// This function has been created in order the withdraw an order that doesn't meet our criteria
// It optimizes the order research by reducing it to the appropriate vector only i.e. the vector that corresponds to the relevant timestamp
void OrderBook::removeOrder(OrderBookEntry &order) {
  // The correct vector is selected
  std::vector<OrderBookEntry> timestampOrders = ordersMap[order.timestamp];
  // We iterate an all vector elements and we remove any orders that have been placed by the bot user
  for (int i = 0; i < timestampOrders.size(); i++) {
    // Condition for finding the bot orders and leaving the rest out
    if (timestampOrders[i].username == "bot") {
      // Erase the element by passing the first element of the vector and the current iterating poistion
      timestampOrders.erase(timestampOrders.begin() + i);
    }
  }
}

std::vector<OrderBookEntry> OrderBook::matchAsksToBids(std::string product,
                                                       std::string timestamp) {
  std::vector<OrderBookEntry> asks =
      getOrders(OrderBookType::ask, product, timestamp);
  std::vector<OrderBookEntry> bids =
      getOrders(OrderBookType::bid, product, timestamp);
  std::vector<OrderBookEntry> sales;

  // I put in a little check to ensure we have bids and asks
  // to process.
  if (asks.size() == 0 || bids.size() == 0) {
    std::cout << " OrderBook::matchAsksToBids no bids or asks" << std::endl;
    return sales;
  }

  /* sort asks in ascending order */
  std::sort(asks.begin(), asks.end(),
            [](const OrderBookEntry &e1, const OrderBookEntry &e2) {
              return e1.price < e2.price;
            });
  /* sort bids in descending order */
  std::sort(bids.begin(), bids.end(),
            [](const OrderBookEntry &e1, const OrderBookEntry &e2) {
              return e1.price > e2.price;
            });

  // This string will take into account which was the context of each transaction 
  // and then storing it into the sale itself
  
  std::string controlString =
      "max ask: " + std::to_string(asks[asks.size() - 1].price) +
      " | min ask: " + std::to_string(asks[0].price) +
      " | max bid: " + std::to_string(bids[0].price) +
      " | min bid: " + std::to_string(bids[bids.size() - 1].price);

  // for ask in asks:
  for (OrderBookEntry &ask : asks) {
    // for bid in bids:
    for (OrderBookEntry &bid : bids) {
      if (bid.price < ask.price) {
        continue;
      }
      if (bid.amount <= 0)
        continue;

      OrderBookEntry sale{ask.price, 0, timestamp, product,
                          OrderBookType::asksale};
      sale.controlString = controlString;

      if (bid.username == "simuser" || bid.username == "bot") {
        sale.username = bid.username;
        sale.orderType = OrderBookType::bidsale;
      }
      if (ask.username == "simuser" || ask.username == "bot") {
        sale.username = ask.username;
        sale.orderType = OrderBookType::asksale;
      }

      // # now work out how much was sold and
      // # create new bids and asks covering
      // # anything that was not sold
      // if bid.amount == ask.amount: # bid completely clears ask
      // Amount that bidded is = amount that was offered
      if (bid.amount == ask.amount) {
        // Our amount is the full ask amount
        sale.amount = ask.amount;
        // sales.append(sale)
        sales.push_back(sale);
        // bid.amount = 0 # make sure the bid is not processed
        // again
        bid.amount = 0;
        // # can do no more with this ask
        // # go onto the next ask
        // break
        break;
      }
      // if bid.amount > ask.amount:  # ask is completely gone slice
      // the bid
      if (bid.amount > ask.amount) {
        // sale.amount = ask.amount
        // Amount that bidded is higher than amount that was offered
        // Our amount is the full ask amount
        sale.amount = ask.amount;
        // sales.append(sale)
        sales.push_back(sale);
        // # we adjust the bid in place
        // # so it can be used to process the next ask
        // bid.amount = bid.amount - ask.amount
        // Get the remaining bid after buying
        bid.amount = bid.amount - ask.amount;
        // # ask is completely gone, so go to next ask
        // break
        break;
      }
      // if bid.amount < ask.amount # bid is completely gone,
      // slice the ask
      if (bid.amount < ask.amount && bid.amount > 0) {
        // sale.amount = bid.amount
        // What was offered is lower than what was asked
        // We are selling at the bid price
        sale.amount = bid.amount;
        // sales.append(sale)
        sales.push_back(sale);
        // # update the ask
        // # and allow further bids to process the remaining
        // amount ask.amount = ask.amount - bid.amount
        ask.amount = ask.amount - bid.amount;
        // bid.amount = 0 # make sure the bid is not processed
        // again
        bid.amount = 0;
        // # some ask remains so go to the next bid
        // continue
        continue;
      }
    }
  }
  return sales;
}
