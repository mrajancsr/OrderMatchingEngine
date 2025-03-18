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

#include "Order.h"
#include <iostream>
#include <unordered_map>
#include <unordered_set>

template <typename T>
void displayOrders(const T &container)
{
    for (const Order &order : container)
    {
        std::cout << "Order( " + order.OrderId() + ", ";
        std::cout << order.SecurityId() + ", " + order.User();
        std::cout << ", " << order.Qty() << ", " << order.Company() << ", " << order.Price() << " ) " << std::endl;
    }
}

class IOrderEngine
{
public:
    virtual void addOrder(const Order &order) = 0;
    virtual std::optional<Order> getOrder(const std::string &orderId) const = 0;
    virtual std::vector<Order> getAllOrders() const = 0;
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

    std::optional<Order> getOrder(const std::string &orderId) const override
    {
        auto itOrder = m_ordersByOrderId.find(orderId);
        if (itOrder != m_ordersByOrderId.end())
            return itOrder->second;
        return std::nullopt;
    }

    std::vector<Order> getAllOrders() const override
    {
        std::vector<Order> orders;
        for (const auto &[orderid, order] : m_ordersByOrderId)
            orders.push_back(order);
        return orders;
    }
};

#endif