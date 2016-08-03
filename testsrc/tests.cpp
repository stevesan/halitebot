
#include <stdlib.h>
#include <iostream>
#include "../src/Int2.hpp"
#include "../src/Grid2.hpp"
#include "../src/GridAlgos.hpp"
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
}
