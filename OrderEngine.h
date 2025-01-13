#include <string>
#include <algorithm>
#include <iostream>
#include <mutex>
#include <set>
#include <unordered_map>
#include <unordered_set>

class Order
{
private:
    std::string m_orderId;
    std::string m_securityId;
    std::string m_side;
    std::string m_user;
    std::string m_company;
    unsigned int m_qty;

public:
    Order(const std::string &ordId,
          const std::string &secId,
          const std::string &side,
          const unsigned int qty,
          const std::string &user,
          const std::string &company)
        : m_orderId{ordId}, m_securityId{secId}, m_side{side}, m_qty{qty}, m_user{user}, m_company{company} {}

    std::string orderId() const { return m_orderId; }
    std::string securityId() const { return m_securityId; }
    unsigned int qty() const { return m_qty; }
    std::string userId() const { return m_user; }
    std::string side() const { return m_side; }

    bool operator==(const Order &otherOrder) const
    {
        return (this->m_orderId == otherOrder.orderId() &&
                (this->securityId() == otherOrder.securityId()) &&
                (this->userId() == otherOrder.userId()) &&
                (this->side() == otherOrder.side()));
    }
};

namespace std
{
    template <>
    struct hash<Order>
    {
        std::size_t operator()(const Order &order) const
        {
            return std::hash<std::string>()(order.orderId()) ^
                   (std::hash<std::string>()(order.securityId()) << 1) ^
                   (std::hash<std::string>()(order.userId()) << 2);
        }
    };
}

class IOrderEngine
{
public:
    virtual void addOrder(const Order &order) = 0;
    virtual void cancelOrder(const std::string &orderId) = 0;
    virtual std::vector<Order> getOrder(const std::string &orderId) = 0;
    virtual void cancelOrderByUser(const std::string &userId) = 0;
    virtual std::unordered_set<Order> getOrdersByUser(const std::string &userId) = 0;
};

class OrderEngine : public IOrderEngine
{
private:
    std::unordered_map<std::string, Order> m_ordersByOrderId;
    std::unordered_map<std::string, std::unordered_set<Order>> m_ordersBySecurityId;
    std::unordered_map<std::string, std::unordered_set<std::string>> m_ordersByUserId;
    mutable std::mutex m_mutex; // mutex to protect shared resources

public:
    void addOrder(const Order &order) override
    {
        std::lock_guard<std::mutex> lock(m_mutex); // lock guard for thread safety
        auto [it, inserted] = m_ordersByOrderId.emplace(order.orderId(), order);

        if (!inserted)
            throw ::std::runtime_error("Duplicate order detected: " + order.orderId());
        m_ordersBySecurityId[order.securityId()].insert(order);
        m_ordersByUserId[order.userId()].insert(order.orderId());
    }

    void cancelOrder(const std::string &orderId) override
    {
        std::lock_guard<std::mutex> lock(m_mutex); // lock guard for thread safety

        auto orderit = m_ordersByOrderId.find(orderId);
        if (orderit != m_ordersByOrderId.end())
        {
            const auto &secId = orderit->second.securityId();
            auto &orders = m_ordersBySecurityId[secId];
            orders.erase(orderit->second);
            if (orders.empty())
                m_ordersBySecurityId.erase(secId);
            const auto &userId = orderit->second.userId();
            auto &userOrders = m_ordersByUserId[userId];
            userOrders.erase(orderId);
            if (userOrders.empty())
                m_ordersByUserId.erase(userId);
            m_ordersByOrderId.erase(orderit);
        }
    }
};