#pragma once
#include "CSVReader.hpp"
#include "OrderBookEntry.hpp"
#include <string>
#include <vector>

class OrderBook {
public:
  /** construct, reading a csv data file */
  OrderBook(std::string filename);
  /** return vector of all know products in the dataset*/
  std::vector<std::string> getKnownProducts();
  /** return vector of Orders according to the sent filters*/
  std::vector<OrderBookEntry> getOrders(OrderBookType type, std::string product,
                                        std::string timestamp);
  /** return vector of Orders according to the sent filters*/
  std::vector<OrderBookEntry> getOrdersByTypeAndProduct(OrderBookType type,
                                                        std::string product);

  /** returns the earliest time in the orderbook*/
  std::string getEarliestTime();
  /** returns the next time after the
   * sent time in the orderbook
   * If there is no next timestamp, wraps around to the start
   * */
  std::string getNextTime(std::string timestamp);
  /** insert order in orderbookentry */
  void insertOrder(OrderBookEntry &order);
  /** remove order from orderbookentry */
  void removeOrder(OrderBookEntry &order);
  /** get the overall size of the orders vector */
  int getOrdersSize();

  std::vector<OrderBookEntry> matchAsksToBids(std::string product,
                                              std::string timestamp);

  /** get the highest price in the registry */
  static double getHighPrice(std::vector<OrderBookEntry> &orders);
  /** get the lowest price in the registry */
  static double getLowPrice(std::vector<OrderBookEntry> &orders);

private:
  std::vector<OrderBookEntry> orders;
  std::map<std::string, std::vector<OrderBookEntry>> ordersMap;
};
