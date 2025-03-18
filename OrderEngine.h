#ifndef ORDER_ENGINE_H
#define ORDER_ENGINE_H

/**
 * @file OrderEngine.h
 * @brief Defines the Order Book for a stock order matching engine
 *
 * This class manages buy and sell orders, matches trades based on various priority rules,
 * and maintains an efficient order book for execution.
 *
 * Features:
 * - Supports Limit Orders (buy/sell)
 * - Matches orders using price-time priority
 * - Prints trade execution details
 * - Supports order cancellations and modifications (future feature)
 *
 * @author Rajan Subramanian
 * @date March 2025
 *
 */

#include <iostream>
#include <unordered_map>
#include <unordered_set>

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
    const double GetQty() const { return m_qty; }
};

class IOrderEngine
{
public:
    virtual void addOrder(const Order &order) = 0;
};

class OrderEngine final : public IOrderEngine
{
private:
    std::unordered_map<std::string, Order> m_ordersByOrderId;
    std::unordered_map<std::string, std::unordered_set<Order>> m_ordersBySecurityId;
    std::unordered_map<std::string, std::unordered_set<std::string>> m_ordersByUserId;

public:
    void addOrder(const Order &order) override
    {
        auto [it, inserted] = m_ordersByOrderId.emplace(order.OrderId(), order);
        if (!inserted)
            throw ::std::runtime_error("Duplicate order detected: " + order.OrderId());
        m_ordersBySecurityId[order.SecurityId()].insert(order);
        m_ordersByUserId[order.User()].insert(order.OrderId());
    }
};

#endif