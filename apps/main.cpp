#include "OrderBook.hpp"
#include <iostream>

int main() {
  OrderBook book;
  std::cout << "best bid: "
            << (book.bestBid() ? std::to_string(*book.bestBid()) : "none")
            << "\n";
}
