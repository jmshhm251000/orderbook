#include "OrderBook.hpp"
#include <limits>

Result OrderBook::AddOrder(OrderId id, OrderInfo info) {
  if (info.volume <= 0)
    return Result::InvalidVolume;

  auto [it, inserted] = mOrders.emplace(id, info);
  if (!inserted)
    return Result::DuplicateOrder;

  AdjustSide(info.side, [&](Levels &levels, Queues &queues, auto comp) {
    Add(levels, queues, id, info.price, info.volume, comp);
  });
  return Result::Ok;
}

Result OrderBook::DeleteOrder(OrderId id) {
  auto it = mOrders.find(id);
  if (it == mOrders.end())
    return Result::UnknownOrder;

  const auto &order = it->second;
  AdjustSide(order.side, [&](Levels &levels, Queues &queues, auto comp) {
    Pop(levels, queues, id, order.price);
  });
  mOrders.erase(it);
  return Result::Ok;
}

Result OrderBook::ModifyOrder(OrderId id, Volume newVolume) {
  auto it = mOrders.find(id);
  if (it == mOrders.end())
    return Result::UnknownOrder;
  if (newVolume <= 0)
    return newVolume == 0 ? DeleteOrder(id) : Result::InvalidVolume;

  auto &order = it->second;
  if (newVolume < order.volume) {
    AdjustSide(order.side, [&](Levels &levels, Queues &queues, auto comp) {
      Modify(levels, queues, id, order.price, newVolume);
    });
  } else {
    AdjustSide(order.side, [&](Levels &levels, Queues &queues, auto comp) {
      Pop(levels, queues, id, order.price);
      Add(levels, queues, id, order.price, newVolume, comp);
    });
  }

  order.volume = newVolume;
  return Result::Ok;
}

Result OrderBook::ReplaceOrder(OrderId id, OrderId newId, Price newPrice,
                               Volume newVolume) {
  auto it = mOrders.find(id);
  if (it == mOrders.end())
    return Result::UnknownOrder;

  return Result::Ok;
}

Price OrderBook::BestBid() const {
  if (mBidLevels.size() == 0)
    return std::numeric_limits<Price>::max();
  return mBidLevels.front().price;
}
