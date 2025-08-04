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
 * - Supports order cancellations and modifications
 *
 * @author Rajan Subramanian
 * @date March 2025
 *
 */

#include "Order.h"
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <mutex>
#include <optional>

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
    virtual bool cancelOrdersForSecIdWithMinimumQty(const std::string &secId, unsigned int minQty) = 0;
    virtual bool modifyOrder(const std::string &orderId, const unsigned int &newQty) = 0;
    virtual std::vector<Order> getAllOrders() const = 0;
    virtual const std::unordered_set<Order> &getOrdersBySecurityId(const std::string &secId) const = 0;
    virtual const std::vector<Order> getOrdersByUserId(const std::string &userId) const = 0;
    virtual std::optional<Order> getOrder(const std::string &orderId) const = 0;
    virtual unsigned int getMatchingSizeForSecurity(const std::string &secId) = 0;
};

class OrderEngine final : public IOrderEngine
{
private:
    std::unordered_map<std::string, Order> m_ordersByOrderId;
    std::unordered_map<std::string, std::unordered_set<Order>> m_ordersBySecurityId;
    std::unordered_map<std::string, std::unordered_set<std::string>> m_ordersByUserId;
    mutable std::mutex m_mutex;

public:
    void addOrder(Order order) override
    {
        const auto &[it, inserted] = m_ordersByOrderId.emplace(order.OrderId(), std::move(order));
        if (!inserted)
            throw std::runtime_error("Duplicate order detected: " + it->first);

        const Order &insertedOrder = it->second;

        m_ordersBySecurityId[insertedOrder.SecurityId()].insert(insertedOrder);
        m_ordersByUserId[insertedOrder.UserId()].insert(insertedOrder.OrderId());
    }

    void cancelOrder(const std::string &orderId) override
    {
        auto itOrder = m_ordersByOrderId.find(orderId);
        if (itOrder == m_ordersByOrderId.end())
            return; // Order doesn't exist, do nothing

        const std::string &secId = itOrder->second.SecurityId();
        const std::string &userId = itOrder->second.UserId();

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
                m_ordersByUserId.erase(itUser);
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

        // cancel each order associated with userId
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

    unsigned int getMatchingSizeForSecurity(const std::string &secId) override
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto secIt = m_ordersBySecurityId.find(secId);

        if (secIt == m_ordersBySecurityId.end())
            return 0;

        std::vector<Order> buys, sells;

        for (const auto &order : secIt->second)
        {
            if (order.Side() == OrderSide::BUY)
                buys.push_back(order);
            else if (order.Side() == OrderSide::SELL)
                sells.push_back(order);
        }

        std::sort(buys.begin(), buys.end(), [](const Order &a, const Order &b)
                  { return a.Price() > b.Price(); });

        std::sort(sells.begin(), sells.end(), [](const Order &a, const Order &b)
                  { return a.Price() < b.Price(); });

        unsigned int matchedQty = 0;
        size_t i = 0, j = 0;

        while (i < buys.size() && j < sells.size())
        {
            Order &buy = buys[i];
            Order &sell = sells[j];

            if (buy.Company() == sell.Company())
            {
                // skip matching same company name
                // advance to which ever side is more likely match
                if ((i + 1 < buys.size() && buys[i + 1].Company() != sell.Company()) || j + 1 == sells.size())
                    ++i;
                else
                    ++j;
                continue;
            }

            // cant match
            /*
            if (buy.Price() < sell.Price())
            {
                ++i;
                continue;
            }*/
            unsigned int qty = std::min(buy.Qty(), sell.Qty());
            matchedQty += qty;
            buy.SetQty(buy.Qty() - qty);
            sell.SetQty(sell.Qty() - qty);

            if (buy.Qty() == 0)
                ++i;
            if (sell.Qty() == 0)
                ++j;
        }
        return matchedQty;
    }
};

#endif