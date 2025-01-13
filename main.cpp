#include "OrderEngine.h"
#include <iostream>

int main()
{
    OrderEngine engine;
    engine.addOrder(Order("ID1", "SecID1", "Buy", 100, "raju", "boa"));
    engine.addOrder(Order("ID2", "SecID1", "Sell", 1000, "raju", "boa"));

    return 0;
}