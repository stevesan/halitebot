
#include <stdlib.h>
#include <iostream>
#include "../src/Int2.hpp"
#include "../src/Grid2.hpp"
#include "../src/GridAlgos.hpp"
#include "../src/Util.hpp"
#include "../src/Range2.hpp"
#include <queue>
#include <set>
#include <algorithm>

#include <cassert>

void testLambdaSort()
{
    std::vector<int> v;
    v.push_back(3);
    v.push_back(1);
    v.push_back(2);

    auto lt = [&] (int a, int b) -> bool { return a < b; };
    std::sort( v.begin(), v.end(), lt );
    assert( v[0] == 1 );
    assert( v[1] == 2 );
    assert( v[2] == 3 );
}

void testInt2()
{
    Int2 u(5,5);

    std::set<Int2> nbors;
    for(Int2 v : Nbors(u)) {
        assert( nbors.find(v) == nbors.end() );
        assert( v != u );
        assert( u.taxiDist(v) == 1 );
        nbors.insert(v);
    }

    assert(nbors.size() == 4);
}

void testRange()
{
    Range2 r1(Int2(0,0), Int2(2,1));

    Int2 seq[] = { Int2(0,0), Int2(1,0) };
    int seqnum = 0;
    for(Int2 u : r1) {
        assert (u == seq[seqnum]);
        seqnum++;
    }

    Range2 r2(Int2(1, 2), Int2(3, 4));
    int count = 0;
    for(Int2 u : r2) { count++; }
    assert( count == 4 );
}

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

    testInt2();
    testRange();
    testLambdaSort();
}
