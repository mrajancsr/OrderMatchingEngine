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
    virtual void addOrder(Order order) = 0;
    virtual std::optional<Order> getOrder(const std::string &orderId) const = 0;
    virtual std::vector<Order> getAllOrders() const = 0;
    virtual std::vector<Order> getOrdersBySecurityId(const std::string &secId) const = 0;
};

class OrderEngine final : public IOrderEngine
{
private:
    std::unordered_map<std::string, Order> m_ordersByOrderId;
    std::unordered_map<std::string, std::unordered_set<Order>> m_ordersBySecurityId;
    std::unordered_map<std::string, std::unordered_set<std::string>> m_ordersByUserId;

public:
    void addOrder(Order order) override
    {
        auto [it, inserted] = m_ordersByOrderId.emplace(order.OrderId(), std::move(order));
        if (!inserted)
            throw ::std::runtime_error("Duplicate order detected: " + it->first);

        const Order &insertedOrder = it->second;
        const std::string userId = insertedOrder.User();
        const std::string orderId = insertedOrder.OrderId();

        m_ordersBySecurityId[insertedOrder.SecurityId()].insert(std::move(insertedOrder));
        m_ordersByUserId[userId].insert(orderId);
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
        orders.reserve(m_ordersByOrderId.size());
        for (const auto &[orderid, order] : m_ordersByOrderId)
            orders.push_back(order);
        return orders; // NRVO since its a named local variable
    }

    virtual std::vector<Order> getOrdersBySecurityId(const std::string &secId) const override
    {
        std::vector<Order> orders;
        auto itOrder = m_ordersBySecurityId.find(secId);
        if (itOrder != m_ordersBySecurityId.end())
        {
            orders.reserve(itOrder->second.size());
            for (const auto &order : itOrder->second)
                orders.push_back(order);
        }
        return orders;
    }
};

#endif