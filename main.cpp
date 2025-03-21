#include "OrderEngine.h"
#include <iostream>

int main()
{
    OrderEngine engine;

    engine.addOrder(Order("ID1", "GOLD", OrderSide::BUY, 100, "alice", "firmA", 1850.5));
    engine.addOrder(Order("ID2", "GOLD", OrderSide::SELL, 50, "bob", "firmB", 1850.5));
    engine.addOrder(Order("ID3", "Corn", OrderSide::SELL, 100, "bob", "firmB", 2000.0));
    engine.addOrder(Order("ID4", "WTI", OrderSide::BUY, 10, "alice", "firmA", 550));
    engine.addOrder(Order("ID5", "WTI", OrderSide::BUY, 30, "alice", "firmA", 548));
    engine.addOrder(Order("ID6", "GOLD", OrderSide::BUY, 100, "alice", "firmA", 1851.3));

    // get all orders
    std::cout << "getting all orders entered" << std::endl;
    const auto &allOrders = engine.getAllOrders();
    displayOrders(allOrders);

    std::cout << "Getting all Orders associated with GOLD" << std::endl;
    const auto &orders = engine.getOrdersBySecurityId("GOLD");
    displayOrders(orders);

    std::cout << "Alice wants to cancel orderId 1: cancelling order..." << std::endl;
    engine.cancelOrder("ID1");
    std::cout << "After cancel:\n";
    const auto &after = engine.getOrdersBySecurityId("GOLD");
    displayOrders(after);

    std::cout << "Getting all Orders associated with Alice" << std::endl;
    const auto &aliceOrders = engine.getOrdersByUserId("alice");
    displayOrders(aliceOrders);
}