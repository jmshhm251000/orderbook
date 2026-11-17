#pragma once
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <unordered_map>
#include <vector>

enum class Side : uint8_t { Bid, Ask };
enum class Result : uint8_t { Ok, UnknownOrder, InvalidVolume, DuplicateOrder };

using OrderId = uint64_t;
using Volume = int64_t;
using Price = int64_t;

struct OrderInfo {
  Side side;
  Price price;
  Volume volume;
};

struct RestingOrder {
  OrderId id;
  Volume volume;
};

struct Level {
  Price price;
  Volume total;
};
using Levels = std::vector<Level>;
using Queue = std::deque<RestingOrder>;
using Queues = std::vector<Queue>;

class OrderBook {
public:
  Result AddOrder(OrderId id, OrderInfo info);
  Result DeleteOrder(OrderId id);
  Result ModifyOrder(OrderId id, Volume volume);
  Result ReplaceOrder(OrderId id, OrderId newId, Price newPrice,
                      Volume newVolume);
  Price BestBid() const;
  Price BestAsk() const;
  Volume VolumeAt(Side side, Price price) const;
  size_t LevelCount(Side side) const;
  size_t OrderCount() const;

private:
  Levels mBidLevels;
  Queues mBidQueues;
  Levels mAskLevels;
  Queues mAskQueues;

  std::unordered_map<OrderId, OrderInfo> mOrders;

  template <class F> decltype(auto) AdjustSide(Side side, F &&f) {
    if (side == Side::Bid)
      return f(mBidLevels, mBidQueues, std::greater<Price>{});
    return f(mAskLevels, mAskQueues, std::less<Price>{});
  }

  template <class Compare>
  void Add(Levels &levels, Queues &queues, OrderId id, Price price,
           Volume volume, Compare comp) {
    auto it = std::find_if(levels.begin(), levels.end(), [&](const auto &p) {
      return !comp(p.price, price);
    });

    size_t i = it - levels.begin();

    if (it != levels.end() && it->price == price) {
      levels[i].total += volume;
      queues[i].push_back({id, volume});
    } else {
      levels.insert(it, Level{price, volume});
      queues.insert(queues.begin() + i, Queue{RestingOrder{id, volume}});
    }
  }

  void Pop(Levels &levels, Queues &queues, OrderId id, Price price) {
    auto itLevels =
        std::find_if(levels.begin(), levels.end(),
                     [&](const auto &p) { return p.price == price; });
    if (itLevels == levels.end())
      return;

    size_t i = itLevels - levels.begin();
    auto &q = queues[i];
    auto itDeque = std::find_if(
        q.begin(), q.end(), [&](const RestingOrder &r) { return r.id == id; });
    if (itDeque == q.end())
      return;

    itLevels->total -= itDeque->volume;
    q.erase(itDeque);
    if (q.empty()) {
      levels.erase(itLevels);
      queues.erase(queues.begin() + i);
    }
  }

  void Modify(Levels &levels, Queues &queues, OrderId id, Price price,
              Volume newVolume) {
    auto itLevels =
        std::find_if(levels.begin(), levels.end(),
                     [&](const auto &p) { return p.price == price; });
    if (itLevels == levels.end())
      return;

    size_t i = itLevels - levels.begin();
    auto &q = queues[i];
    auto itDeque = std::find_if(
        q.begin(), q.end(), [&](const RestingOrder &r) { return r.id == id; });
    if (itDeque == q.end())
      return;

    const auto delta = newVolume - itDeque->volume;
    itLevels->total += delta;
    itDeque->volume = newVolume;
  }
};
