#include "OrderEngine.h"
#include <iostream>

int main()
{
    OrderEngine engine;

    engine.addOrder(Order("ID1", "SecID1", OrderSide::BUY, 100, "raju", "boa", 50));
    engine.addOrder(Order("ID2", "SecID1", OrderSide::SELL, 1000, "raju", "boa", 30));
    // engine.addOrder(Order("ID3", "SecID2", OrderSide::BUY, 500, "prema", "rush", 10));
    // engine.addOrder(Order("ID4", "SecID3", OrderSide::BUY, 600, "prema", "rush", 55.5));
    // engine.addOrder(Order("ID5", "SecID4", OrderSide::BUY, 300, "raju", "boa", 56.0));
    //  engine.addOrder(Order("ID6", "SecID2", OrderSide::SELL, 400, "neptune", "kernel"));
    //  engine.addOrder(Order("ID7", "SecID3", OrderSide::SELL, 500, "neptune", "kernel"));
    // engine.addOrder(Order("ID8", "SecID1", OrderSide::SELL, 300, "neptune", "kernel", 66.7));
    // engine.addOrder(Order("ID9", "SecID2", OrderSide::BUY, 500, "neptune", "kernel", 45.6));

    const auto orders = engine.getAllOrders();
    displayOrders(orders);

    

    return 0;
}