#pragma once

#include "OrderBookEntry.hpp"
#include <map>
#include <string>
#include <vector>

class CSVReader {
public:
  CSVReader();
  /** generate a vector of entries as read from the source .csv  */
  static std::vector<OrderBookEntry> readCSV(std::string csvFile);
  static std::map<std::string, std::vector<OrderBookEntry>>
  readCSVMap(std::string csvFile);
  /** split the csv line based on a separator character */
  static std::vector<std::string> tokenise(std::string csvLine, char separator);
  /** transform tokenized strings into an obe  */
  static OrderBookEntry stringsToOBE(std::string price, std::string amount,
                                     std::string timestamp, std::string product,
                                     OrderBookType OrderBookType);

private:
  static OrderBookEntry stringsToOBE(std::vector<std::string> strings);
};
