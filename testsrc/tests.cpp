
#include <stdlib.h>
#include <iostream>
#include "../src/Int2.hpp"
#include "../src/Grid2.hpp"
#include "../src/GridAlgos.hpp"
#include "../src/Util.hpp"
#include <queue>

#include <cassert>

int main(void) {
    Grid2<int> g(Int2(10,10));
	srand(123);

    int expectedSum = 0;
    g.foreachKey([&](Int2 u) {
            int val = rand() % 100;
            expectedSum += val;
            g.set(u, val);
            });

    assert(expectedSum == g.sum());

    std::cout << "g contents: " << std::endl;
    g.writeCsv(std::cout);

    g.set(Int2(5,5), 0);
    std::queue<Int2> changed;
    g.foreachPair([&](Int2 u, int val) {
            if( val == 0 ) {
                changed.push(u);
            }
            });
    updateDistanceField( g, changed );

    std::cout << "g contents: " << std::endl;
    g.writeCsv(std::cout);


    // utils
    int nums[] = {2, -1, 3, -2};

    int act;

    auto id = [] (int x) { return x; };
    auto neg = [] (int x) { return -1 * x; };
    auto myabs = [] (int x) { return (int)std::abs(x); };

    act = minKey<int,int>(nums, 4, id);
    assert(act == -2);

    act = minKey<int,int>(nums, 4, neg);
    assert(act == -3 );

    act = minKey<int,int>(nums, 4, myabs);
    assert(act == 1);

    act = findMin<int,int>(nums, 4, id);
    assert(act == 3);

    act = findMin<int,int>(nums, 4, neg);
    assert(act == 2 );

    act = findMin<int,int>(nums, 4, myabs);
    assert(act == 1);

    act = findMax<int,int>(nums, 4, id);
    assert(act == 2);

    act = findMax<int,int>(nums, 4, neg);
    assert(act == 3);

    act = findMax<int,int>(nums, 4, myabs);
    assert(act == 2);
}
