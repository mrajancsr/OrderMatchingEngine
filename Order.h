#ifndef ORDER_H
#define ORDER_H

#include <string>
#include <ostream>

enum class OrderSide
{
    BUY,
    SELL
};

class Order
{
private:
    std::string m_orderId;
    std::string m_securityId;
    OrderSide m_side;
    std::string m_user;
    std::string m_company;
    unsigned int m_qty;
    double m_price;
    std::string orderInString;

public:
    Order(const std::string orderId,
          const std::string secId,
          OrderSide side,
          const unsigned int qty,
          const std::string user,
          const std::string company,
          const double price)
        : m_orderId{std::move(orderId)}, m_securityId{std::move(secId)}, m_side{std::move(side)},
          m_user{std::move(user)}, m_company{std::move(company)}, m_qty{qty}, m_price{price} {};

    // copy structure (default)
    Order(const Order &) = default;

    // Move constructor
    Order(Order &&other) noexcept
        : m_orderId{std::move(other.m_orderId)},
          m_securityId{std::move(other.m_securityId)},
          m_side{std::move(other.m_side)},
          m_user{std::move(other.m_user)},
          m_company{std::move(other.m_company)},
          m_qty{std::move(other.m_qty)},
          m_price{std::move(other.m_price)} {}

    // copy assignment (default)
    Order &operator=(const Order &) = default;

    // Move assignment
    Order &operator=(Order &&other) noexcept
    {
        if (this != &other)
        {
            m_orderId = std::move(other.m_orderId);
            m_securityId = std::move(other.m_securityId);
            m_side = std::move(other.m_side);
            m_user = std::move(other.m_user);
            m_company = std::move(other.m_company);
            m_qty = std::move(other.m_qty);
            m_price = std::move(other.m_price);
        }
        return *this;
    }

    // Overload the << operator
    friend std::ostream &operator<<(std::ostream &os, const Order &order);

    const std::string &OrderId() const { return m_orderId; }
    const std::string &SecurityId() const { return m_securityId; }
    const std::string &UserId() const { return m_user; }
    const std::string &Company() const { return m_company; }
    const OrderSide &Side() const { return m_side; }
    const double Qty() const { return m_qty; }
    const double Price() const { return m_price; }
    void SetQty(const unsigned int &newQty) { m_qty = newQty; }

    bool operator==(const Order &otherOrder) const
    {
        return (this->m_orderId == otherOrder.OrderId() &&
                (this->SecurityId() == otherOrder.SecurityId()) &&
                (this->UserId() == otherOrder.UserId()) &&
                (this->Side() == otherOrder.Side()));
    }
};

std::ostream &operator<<(std::ostream &os, const Order &order)
{
    os << "Order(" << order.m_orderId << ", " << order.m_securityId << ", ";
    os << (order.m_side == OrderSide::BUY ? "BUY" : "SELL") << ", ";
    os << order.m_user << ", " << order.m_company << ", " << order.m_qty << ", " << order.m_price << ")";
    return os;
}

namespace std
{
    template <>
    struct hash<Order>
    {
        std::size_t operator()(const Order &order) const
        {
            return std::hash<std::string>()(order.OrderId()) ^
                   (std::hash<std::string>()(order.SecurityId()) << 1) ^
                   (std::hash<std::string>()(order.UserId()) << 2);
        }
    };
}

struct BuyOrderComparator
{
    bool operator()(const Order &a, const Order &b) const
    {
        if (a.Qty() == b.Qty())
        {
            return a.OrderId() < b.OrderId(); // Tie-breaker by orderId
        }
        return a.Qty() > b.Qty(); // Descending order for Buy
    }
};

struct SellOrderComparator
{
    bool operator()(const Order &a, const Order &b) const
    {
        if (a.Qty() == b.Qty())
        {
            return a.OrderId() < b.OrderId(); // Tie-breaker by orderId
        }
        return a.Qty() < b.Qty(); // Ascending order for Sell
    }
};

#endif