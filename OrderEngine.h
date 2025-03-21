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
        std::cout << order << std::endl;
    std::cout << std::endl;
}

class IOrderEngine
{
public:
    virtual void addOrder(Order order) = 0;
    virtual void cancelOrder(const std::string &orderId) = 0;
    virtual void cancelOrderByUser(const std::string &userId) = 0;
    virtual void cancelAllOrdersForSecurity(const std::string &securityId) = 0;
    virtual bool modifyOrder(const std::string &orderId, const unsigned int &newQty) = 0;
    virtual bool cancelOrdersForSecIdWithMinimumQty(const std::string &secId, unsigned int minQty) = 0;
    virtual std::optional<Order> getOrder(const std::string &orderId) const = 0;
    virtual std::vector<Order> getAllOrders() const = 0;
    virtual const std::unordered_set<Order> &getOrdersBySecurityId(const std::string &secId) const = 0;
    virtual const std::vector<Order> getOrdersByUserId(const std::string &userId) const = 0;
    virtual unsigned int getMatchingSizeForSecurity(const std::string &securityId) = 0;
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
        const std::string userId = insertedOrder.UserId();
        const std::string orderId = insertedOrder.OrderId();

        m_ordersBySecurityId[insertedOrder.SecurityId()].insert(std::move(insertedOrder));
        m_ordersByUserId[userId].insert(orderId);
    }

    void cancelOrder(const std::string &orderId) override
    {
        auto itOrder = m_ordersByOrderId.find(orderId);
        if (itOrder == m_ordersByOrderId.end())
            return; // Order doesn't exist, do nothing

        const std::string secId = itOrder->second.SecurityId();
        const std::string userId = itOrder->second.UserId();

        // remove from m_ordersBySecurityId
        auto itSec = m_ordersBySecurityId.find(secId);
        if (itSec != m_ordersBySecurityId.end())
        {
            itSec->second.erase(itOrder->second);
            if (itSec->second.empty())
                m_ordersBySecurityId.erase(itSec);
        }

        // erase from m_ordersByUserId
        auto itUser = m_ordersByUserId.find(userId);
        if (itUser != m_ordersByUserId.end())
        {
            itUser->second.erase(orderId);
            if (itUser->second.empty())
                m_ordersByUserId.erase(userId);
        }
        // finally remove from m_ordersByOrderId
        m_ordersByOrderId.erase(itOrder);

        std::cout << "Order cancelled for orderId: " << orderId << std::endl;
    }

    void cancelOrderByUser(const std::string &userId) override
    {
        auto userIt = m_ordersByUserId.find(userId);
        if (userIt == m_ordersByUserId.end())
            return;

        for (const auto &orderId : userIt->second)
        {
            auto orderIt = m_ordersByOrderId.find(orderId);
            if (orderIt != m_ordersByOrderId.end())
            {
                const std::string &secId = orderIt->second.SecurityId();
                auto secIt = m_ordersBySecurityId.find(secId);
                if (secIt != m_ordersBySecurityId.end())
                {
                    secIt->second.erase(orderIt->second);
                    if (secIt->second.empty())
                        m_ordersBySecurityId.erase(secIt);
                }
                m_ordersByOrderId.erase(orderIt);
            }
        }
        m_ordersByUserId.erase(userIt);
    }

    void cancelAllOrdersForSecurity(const std::string &securityId) override
    {
        auto secIt = m_ordersBySecurityId.find(securityId);
        if (secIt == m_ordersBySecurityId.end())
            return;

        for (const auto &order : secIt->second)
        {
            const auto &orderId = order.OrderId();
            const auto &userId = order.UserId();

            m_ordersByOrderId.erase(orderId);
            auto userIt = m_ordersByUserId.find(userId);
            if (userIt != m_ordersByUserId.end())
            {
                userIt->second.erase(orderId);
                if (userIt->second.empty())
                    m_ordersByUserId.erase(userIt);
            }
        }
        m_ordersBySecurityId.erase(secIt);
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

    const std::unordered_set<Order> &getOrdersBySecurityId(const std::string &secId) const override
    {
        static const std::unordered_set<Order> emptySet;
        auto itOrder = m_ordersBySecurityId.find(secId);
        return (itOrder != m_ordersBySecurityId.end()) ? itOrder->second : emptySet;
    }

    const std::vector<Order> getOrdersByUserId(const std::string &userId) const override
    {
        std::vector<Order> orders;
        auto userIt = m_ordersByUserId.find(userId);
        if (userIt == m_ordersByUserId.end())
            return orders;
        orders.reserve(userIt->second.size());
        for (const auto &orderId : userIt->second)
        {
            const auto &optionalOrder = getOrder(orderId);
            if (optionalOrder)
                orders.push_back(*optionalOrder);
        }
        return orders;
    }

    bool modifyOrder(const std::string &orderId, const unsigned int &newQty) override
    {
        auto orderIt = m_ordersByOrderId.find(orderId);
        if (orderIt == m_ordersByOrderId.end())
            return false;

        // modify from m_ordersBySecurityId
        auto secIt = m_ordersBySecurityId.find(orderIt->second.SecurityId());

        if (secIt == m_ordersBySecurityId.end())
            return false;

        auto &orders = secIt->second;
        orders.erase(orderIt->second);
        orderIt->second.SetQty(newQty);
        orders.insert(orderIt->second);
        return true;
    }

    bool cancelOrdersForSecIdWithMinimumQty(const std::string &secId, unsigned int minQty) override
    {
        auto secIt = m_ordersBySecurityId.find(secId);
        if (secIt == m_ordersBySecurityId.end())
            return false;

        bool orderCancelled = false;

        const auto &orders = secIt->second;
        std::vector<std::string> orderIdsToCancel;
        for (const auto &order : orders)
        {
            if (order.Qty() < minQty)
                orderIdsToCancel.push_back(order.OrderId());
        }
        for (const auto &orderId : orderIdsToCancel)
        {
            cancelOrder(orderId);
            orderCancelled = true;
        }

        return orderCancelled;
    }

    // return total qty that can match for a given securityId
    unsigned int getMatchingSizeForSecurity(const std::string &securityId) override
    {
        auto secIt = m_ordersBySecurityId.find(securityId);
        if (secIt == m_ordersBySecurityId.end())
            return 0;
        return 1;
    }
};

#endif