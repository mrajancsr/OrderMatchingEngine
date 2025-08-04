#include "OrderEngine.h"
#include <iostream>

int main()
{
    OrderEngine engine;

    engine.addOrder(Order("ID1", "GOLD", OrderSide::BUY, 1000, "alice", "firmA", 1850.5));
    engine.addOrder(Order("ID2", "SILVER", OrderSide::SELL, 3000, "bob", "firmB", 1860.5));
    engine.addOrder(Order("ID3", "GOLD", OrderSide::SELL, 500, "janice", "firmA", 2000.0));
    engine.addOrder(Order("ID4", "SILVER", OrderSide::BUY, 600, "steve", "firmC", 550));
    engine.addOrder(Order("ID5", "SILVER", OrderSide::BUY, 100, "raju", "firmB", 548));
    engine.addOrder(Order("ID6", "WTI", OrderSide::BUY, 1000, "appa", "firmD", 1851.3));
    engine.addOrder(Order("ID7", "SILVER", OrderSide::BUY, 2000, "amma", "firmE", 1851.3));
    engine.addOrder(Order("ID8", "SILVER", OrderSide::SELL, 5000, "lenka", "firmE", 1851.3));

    // get all orders
    std::cout << "getting all orders entered" << std::endl;
    const auto &allOrders = engine.getAllOrders();
    displayOrders(allOrders);

    std::cout << "Getting all Orders associated with GOLD" << std::endl;
    const auto &orders = engine.getOrdersBySecurityId("GOLD");
    displayOrders(orders);

    std::cout << "Testing getMatchingSizeForSecurity()" << std::endl;
    std::string secId = "GOLD";
    unsigned int matchedQty = engine.getMatchingSizeForSecurity(secId);
    std::cout << "total matched qty for " << secId << " is " << matchedQty << std::endl;

    std::cout << "Alice wants to cancel orderId 1: cancelling order..." << std::endl;
    engine.cancelOrder("ID1");
    std::cout << "After cancel:\n";
    const auto &after = engine.getOrdersBySecurityId("GOLD");
    displayOrders(after);

    std::cout << "Getting all Orders associated with Alice" << std::endl;
    const auto &aliceOrders = engine.getOrdersByUserId("alice");
    displayOrders(aliceOrders);

    return 0;
}