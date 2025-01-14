#include "OrderEngine.h"
#include <iostream>

int main()
{
    OrderEngine engine;
    engine.addOrder(Order("ID1", "SecID1", "Buy", 100, "raju", "boa", 50));
    engine.addOrder(Order("ID2", "SecID1", "Sell", 1000, "raju", "boa", 30));
    engine.addOrder(Order("ID3", "SecID2", "Buy", 500, "prema", "rush", 10));
    engine.addOrder(Order("ID4", "SecID3", "Buy", 600, "prema", "rush", 55.5));
    engine.addOrder(Order("ID5", "SecID4", "Buy", 300, "raju", "boa", 56.0));
    engine.addOrder(Order("ID6", "SecID2", "Sell", 400, "neptune", "kernel"));
    engine.addOrder(Order("ID7", "SecID3", "Sell", 500, "neptune", "kernel"));
    engine.addOrder(Order("ID8", "SecID1", "Sell", 300, "neptune", "kernel", 66.7));
    engine.addOrder(Order("ID9", "SecID2", "Buy", 500, "neptune", "kernel", 45.6));

    auto allOrders = engine.getAllOrders();
    std::cout << "There are a total of " << allOrders.size() << " Orders" << std::endl;
    displayOrders(allOrders);
    std::cout << std::endl;
    std::cout << "Getting all Orders by raju" << std::endl;
    auto orders = engine.getOrdersByUser("raju");
    if (!orders.empty())
        std::cout << "Total Orders by Raju: " << orders.size() << std::endl;
    displayOrders(orders);
    std::cout << "cancelling orders by raju" << std::endl;
    engine.cancelOrderByUser("raju");
    orders = engine.getOrdersByUser("raju");
    if (!orders.empty())
        std::cout << "Total Orders by Raju: " << orders.size() << std::endl;
    else
        std::cout << "No Orders by raju" << std::endl;
    allOrders = engine.getAllOrders();
    displayOrders(allOrders);
    std::cout << "cancelng orders with secid2" << std::endl;
    std::cout << "modifying order for ID9 which is for neptune user" << std::endl;
    engine.modifyOrder("ID9", 100);
    allOrders = engine.getAllOrders();
    displayOrders(allOrders);
    std::cout << "displaying orders by user neptune" << std::endl;
    orders = engine.getOrdersByUser("neptune");
    displayOrders(orders);

    std::cout << "displaying orders by secId ID2" << std::endl;
    orders = engine.getOrdersBySecurityId("SecID2");
    displayOrders(orders);

    return 0;
}