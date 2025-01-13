#include <string>
#include <algorithm>
#include <iostream>
#include <mutex>
#include <set>
#include <unordered_map>

class Order
{
private:
    std::string m_orderId;
    std::string m_securityId;
    std::string m_side;
    unsigned int m_qty;
    std::string m_user;
    std::string m_company;

public:
    Order(const std::string &ordId,
          const std::string &secId,
          const std::string &side,
          const unsigned int qty,
          const std::string &user,
          const std::string &company)
        : m_orderId{ordId}, m_securityId{secId}, m_side{side}, m_qty{qty}, m_user{user}, m_company{company} {}
};