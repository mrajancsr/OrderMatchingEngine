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
    double m_price;

public:
    Order(const std::string &ordId,
          const std::string &secId,
          const std::string &side,
          const unsigned int qty,
          const std::string &user,
          const std::string &company,
          const double &price = 0.0)
        : m_orderId{ordId}, m_securityId{secId}, m_side{side}, m_qty{qty}, m_user{user}, m_company{company}, m_price{price} {}

    std::string orderId() const { return m_orderId; }
    std::string securityId() const { return m_securityId; }
    unsigned int qty() const { return m_qty; }
    std::string user() const { return m_user; }
    std::string side() const { return m_side; }
    std::string company() const { return m_company; }
    double price() const { return m_price; }
    void setQty(const unsigned int &qty) { m_qty = qty; }

    bool operator==(const Order &otherOrder) const
    {
        return (this->m_orderId == otherOrder.orderId() &&
                (this->securityId() == otherOrder.securityId()) &&
                (this->user() == otherOrder.user()) &&
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
                   (std::hash<std::string>()(order.user()) << 2);
        }
    };
}

class IOrderEngine
{
public:
    virtual void addOrder(const Order &order) = 0;
    virtual void cancelOrder(const std::string &orderId) = 0;
    virtual std::optional<Order> getOrder(const std::string &orderId) const = 0;
    virtual void cancelOrderByUser(const std::string &userId) = 0;

    // get all orders for this particular user
    virtual std::vector<Order> getOrdersByUser(const std::string &userId) const = 0;

    // get all active orders in the engine
    virtual std::vector<Order> getAllOrders() const = 0;

    // remove all orders in engine for this security with qty >= minQty
    virtual void cancelOrdersForSecIdWithMinimumQty(const std::string &secId, unsigned int minQty) = 0;

    // return the total qty that can match for the security id
    virtual unsigned int getMatchingSizeForSecurity(const std::string &securityId) = 0;

    virtual void modifyOrder(const std::string &orderId, const unsigned int &newQty) = 0;
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
        m_ordersByUserId[order.user()].insert(order.orderId());
    }

    void cancelOrder(const std::string &orderId) override
    {
        auto orderit = m_ordersByOrderId.find(orderId);
        if (orderit != m_ordersByOrderId.end())
        {
            const auto &secId = orderit->second.securityId();
            const auto &userId = orderit->second.user();

            // remove from m_ordersBySecurityId
            auto &orders = m_ordersBySecurityId[secId];
            orders.erase(orderit->second);
            if (orders.empty())
                m_ordersBySecurityId.erase(secId);

            // erase from m_ordersByUserId
            auto &userOrders = m_ordersByUserId[userId];
            userOrders.erase(orderId);

            if (userOrders.empty())
                m_ordersByUserId.erase(userId);

            m_ordersByOrderId.erase(orderit);
            std::cout << "order cancelled for orderid: " << orderId << std::endl;
        }
    }
    std::optional<Order> getOrder(const std::string &orderId) const override
    {
        auto orderFound = m_ordersByOrderId.find(orderId);
        if (orderFound != m_ordersByOrderId.end())
            return orderFound->second;
        return std::nullopt;
    }

    void cancelOrderByUser(const std::string &userId) override
    {
        std::lock_guard<std::mutex> lock(m_mutex); // lock guard for thread safety
        auto userit = m_ordersByUserId.find(userId);
        if (userit != m_ordersByUserId.end())
        {
            // delete all the order ids associated with this user
            auto orders = userit->second;
            for (const auto &orderId : orders)
                cancelOrder(orderId);
        }
    }

    std::vector<Order> getOrdersByUser(const std::string &userId) const override
    {
        std::lock_guard<std::mutex> lock(m_mutex); // lock guard for thread safety
        std::vector<Order> orders;
        auto orderit = m_ordersByUserId.find(userId);
        if (orderit != m_ordersByUserId.end())
        {
            auto &orderIds = orderit->second;
            for (const auto &orderId : orderIds)
            {
                const auto &optionalOrder = getOrder(orderId);
                if (optionalOrder)
                    orders.push_back(*optionalOrder);
            }
        }
        return orders;
    }

    std::vector<Order> getAllOrders() const override
    {

        std::vector<Order> orders;
        for (auto &[orderId, order] : m_ordersByOrderId)
            orders.push_back(order);

        return orders;
    }

    void cancelOrdersForSecIdWithMinimumQty(const std::string &securityId, unsigned int minQty) override
    {
        std::lock_guard<std::mutex> lock(m_mutex); // lock guard for thread safety
        auto secit = m_ordersBySecurityId.find(securityId);
        if (secit != m_ordersBySecurityId.end())
        {
            auto orders = secit->second;
            for (const auto &order : orders)
            {
                if (order.qty() >= minQty)
                    cancelOrder(order.orderId());
            }
        }
    }

    unsigned int getMatchingSizeForSecurity(const std::string &securityId) override
    {
        return 0;
    }

    // allows to modify existing order
    void modifyOrder(const std::string &orderId, const unsigned int &newQty) override
    {
        auto orderit = m_ordersByOrderId.find(orderId);
        if (orderit == m_ordersByOrderId.end())
            throw std::runtime_error("Order with OrderId " + orderId + " doesn't exist");
        auto &oldOrder = orderit->second;
        oldOrder.setQty(newQty);
        auto &orders = m_ordersBySecurityId[oldOrder.securityId()];
        auto it = orders.find(oldOrder);
        }
};

template <typename T>
void displayOrders(const T &container)
{
    for (const Order &order : container)
    {
        std::cout << "Order( " + order.orderId() + ", ";
        std::cout << order.securityId() + ", " + order.user();
        std::cout << ", " << order.qty() << ", " << order.company() << ", " << order.price() << " ) " << std::endl;
    }
}