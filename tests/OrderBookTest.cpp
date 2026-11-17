#include "OrderBook.hpp"
#include <gtest/gtest.h>

TEST(OrderBook, EmptyBookHasNoBestBidOrAsk) {
  OrderBook book;
  EXPECT_FALSE(book.bestBid().has_value());
  EXPECT_FALSE(book.bestAsk().has_value());
}
