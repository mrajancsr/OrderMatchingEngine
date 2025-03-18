#ifndef ORDER_H
#define ORDER_H

#include <string>

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

    const std::string &OrderId() const { return m_orderId; }
    const std::string &SecurityId() const { return m_securityId; }
    const std::string &User() const { return m_user; }
    const std::string &Company() const { return m_company; }
    const OrderSide &Side() const { return m_side; }
    const double Qty() const { return m_qty; }
    const double Price() const { return m_price; }

    bool operator==(const Order &otherOrder) const
    {
        return (this->m_orderId == otherOrder.OrderId() &&
                (this->SecurityId() == otherOrder.SecurityId()) &&
                (this->User() == otherOrder.User()) &&
                (this->Side() == otherOrder.Side()));
    }
};

namespace std
{
    template <>
    struct hash<Order>
    {
        std::size_t operator()(const Order &order) const
        {
            return std::hash<std::string>()(order.OrderId()) ^
                   (std::hash<std::string>()(order.SecurityId()) << 1) ^
                   (std::hash<std::string>()(order.User()) << 2);
        }
    };
}

#endif