
#include <stdlib.h>
#include <iostream>
#include "../src/Int2.hpp"
#include "../src/Grid2.hpp"
#include "../src/GridAlgos.hpp"
#include "../src/Util.hpp"
#include "../src/hlt.hpp"
#include "../src/Range2.hpp"
#include "../src/Capture.hpp"
#include <queue>
#include <set>
#include <algorithm>

#include <cassert>
#include <unordered_map>

template< typename T >
std::ostream& operator<<( std::ostream& os, std::set<T> S ) {
    os << "{";
    for( T e : S ) {
        os << e << ",";
    }
    os << "}";
    return os;
}


struct TestMap
{
public:

    Grid2<hlt::PlayerId> owner;
    Grid2<int> str;
    Grid2<int> prod;
    Grid2<char> labels;

    hlt::Site getSite(Int2 u) const {
        return hlt::Site({ owner.get(u),
                (unsigned char)str.get(u),
                (unsigned char)prod.get(u)});
    }

    bool isOwned(Int2 u) const {
        return owner.get(u) == 1;
    }

    void output_plan( std::ostream& os, Int2 target, const CapturePlan& plan ) {
        os << "target = " << labels.get(target) << ", s/p = " << str.get(target) << "/" << prod.get(target) << std::endl;
        os << plan.turns << " turns. Positions used:" << std::endl;
        for(int y : str.yy()) {
            for(int x : str.xx()) {
                Int2 u = Int2(x,y);
                if( u == target ) {
                    os << str.get(u) << "*";
                }
                else if( plan.positions.find(u) != plan.positions.end() ) {
                    os << str.get(u) << prod.get(u);
                }
                else {
                    os << "..";
                }
                os << " ";
            }
            os << std::endl;
        }
    }

    Int2 size() const { return str.size(); }

    Int2 find_label(char q) {
        for( Int2 u : labels.indices() ) {
            if( labels.get(u) == q ) {
                return u;
            }
        }
        return Int2(0,0);
    }

    bool run_test(char target_label, CapturePlan& plan) {
        Int2 target = find_label(target_label);
        bool possible = compute_capture_plan(*this, target, plan);
        output_plan(std::cout, target, plan);
        hlt::MoveSet moves;
        output_moves(plan, target, size(), moves);
        assert(moves.size() == plan.positions.size());

        // number of non-stills should not exceed last wave size
        int num_advances = 0;
        for( auto move : moves ) {
            if( move.dir != STILL) {
                num_advances++;
            }
        }
        if( plan.last_wave_produces ) {
            assert(num_advances == 0);
        }
        else {
            assert(num_advances == plan.last_wave.size());
        }
        return possible;
    }
};

void testCapture0()
{
    const int S = 5;

    TestMap map;

    map.labels.reset(S, {
            'A', 'B', 'C', 'D', 'E',
            'F', 'G', 'H', 'I', 'J',
            'K', 'L', 'M', 'N', 'O',
            'P', 'Q', 'R', 'S', 'T',
            'U', 'V', 'W', 'X', 'Y'});

    map.owner.reset(S, {
            0, 0, 0, 0, 0,
            0, 0, 1, 0, 0,
            0, 1, 1, 1, 0,
            0, 0, 1, 0, 0,
            0, 0, 0, 0, 0
            });

    map.str.reset(S, {
            0, 0, 5, 0, 0,
            0, 0, 0, 0, 0,
            0, 2, 0, 0, 999,
            0, 2, 1, 0, 0,
            0, 0, 0, 0, 0
            });

    map.prod.reset(S, {
            0, 0, 0, 0, 0,
            0, 0, 2, 0, 0,
            0, 2, 2, 2, 0,
            0, 0, 2, 0, 0,
            0, 0, 0, 0, 0
            });

    CapturePlan plan;
    bool ok;

    ok = map.run_test('C', plan);
    assert(ok);
    assert(plan.turns == 3);
    assert(plan.positions.size() == 2);

    // impossible
    ok = map.run_test('O', plan);
    assert(!ok);

    ok = map.run_test('Q', plan);
    assert(ok);
    assert(plan.turns == 1);
    assert(plan.positions.size() == 2);

}

void testCapture1()
{
    const int S = 5;

    TestMap map;

    map.labels.reset(S, {
            'A', 'B', 'C', 'D', 'E',
            'F', 'G', 'H', 'I', 'J',
            'K', 'L', 'M', 'N', 'O',
            'P', 'Q', 'R', 'S', 'T',
            'U', 'V', 'W', 'X', 'Y'});

    map.owner.reset(S, {
            0, 0, 1, 0, 0,
            0, 1, 1, 1, 0,
            0, 0, 0, 0, 0,
            0, 0, 0, 0, 0,
            0, 0, 0, 0, 0
            });

    map.str.reset(S, {
            0, 0, 2, 0, 0,
            0, 2, 2, 1, 0,
            0, 0, 5, 0, 0,
            0, 0, 0, 0, 0,
            0, 0, 0, 0, 0
            });

    map.prod.reset(S, {
            0, 0, 0, 0, 0,
            0, 0, 0, 0, 0,
            0, 0, 0, 0, 0,
            0, 0, 0, 0, 0,
            0, 0, 0, 0, 0
            });

    CapturePlan plan;
    bool ok;

    ok = map.run_test('M', plan);
    assert(ok);
    assert(plan.turns == 2);
    assert(plan.positions.size() == 3);
}

void testCaptureWrap()
{
    const int S = 5;

    TestMap map;

    map.labels.reset(S, {
            'A', 'B', 'C', 'D', 'E',
            'F', 'G', 'H', 'I', 'J',
            'K', 'L', 'M', 'N', 'O',
            'P', 'Q', 'R', 'S', 'T',
            'U', 'V', 'W', 'X', 'Y'});

    map.owner.reset(S, {
            1, 0, 0, 0, 0,
            0, 0, 0, 0, 0,
            0, 0, 0, 0, 0,
            1, 0, 0, 0, 0,
            0, 1, 0, 0, 1
            });

    map.str.reset(S, {
            2, 0, 0, 0, 0,
            0, 0, 0, 0, 0,
            0, 0, 0, 0, 0,
            1, 0, 0, 0, 0,
            5, 2, 0, 0, 2
            });

    map.prod.reset(S, {
            0, 0, 0, 0, 0,
            0, 0, 0, 0, 0,
            0, 0, 0, 0, 0,
            0, 0, 0, 0, 0,
            0, 0, 0, 0, 0
            });

    CapturePlan plan;
    bool ok;

    ok = map.run_test('U', plan);
    assert(ok);
    assert(plan.turns == 1);
    assert(plan.positions.size() == 3);
}

void testCaptureMultiChain()
{
    const int S = 5;

    TestMap map;

    map.labels.reset(S, {
            'A', 'B', 'C', 'D', 'E',
            'F', 'G', 'H', 'I', 'J',
            'K', 'L', 'M', 'N', 'O',
            'P', 'Q', 'R', 'S', 'T',
            'U', 'V', 'W', 'X', 'Y'});

    map.owner.reset(S, {
            0, 0, 1, 0, 0,
            1, 0, 1, 0, 1,
            1, 1, 0, 1, 1,
            1, 0, 1, 0, 1,
            0, 0, 1, 0, 0
            });

    map.str.reset(S, {
            0, 0, 1, 0, 0,
            1, 0, 1, 0, 1,
            1, 1, 9, 0, 1,
            1, 0, 0, 0, 1,
            0, 0, 1, 0, 0
            });

    map.prod.reset(S, {
            0, 0, 2, 0, 0,
            0, 0, 0, 0, 0,
            2, 0, 0, 1, 0,
            0, 0, 0, 0, 0,
            0, 0, 0, 0, 0
            });

    CapturePlan plan;
    bool ok;

    ok = map.run_test('M', plan);
    assert(ok);
    assert(plan.turns == 3);
    assert(plan.positions.size() == 6);
    assert(plan.last_wave.size() == 2);
    assert(plan.last_wave_produces);
}

void testCaptureMultiChain2()
{
    const int S = 5;

    TestMap map;

    map.labels.reset(S, {
            'A', 'B', 'C', 'D', 'E',
            'F', 'G', 'H', 'I', 'J',
            'K', 'L', 'M', 'N', 'O',
            'P', 'Q', 'R', 'S', 'T',
            'U', 'V', 'W', 'X', 'Y'});

    map.owner.reset(S, {
            0, 0, 1, 0, 0,
            1, 0, 1, 0, 1,
            1, 1, 0, 1, 1,
            1, 0, 1, 0, 1,
            0, 0, 1, 0, 0
            });

    map.str.reset(S, {
            0, 0, 3, 0, 0,
            1, 0, 1, 0, 1,
            4, 1, 9, 0, 1,
            1, 0, 0, 0, 1,
            0, 0, 1, 0, 0
            });

    map.prod.reset(S, {
            0, 0, 2, 0, 0,
            0, 0, 0, 0, 0,
            2, 0, 0, 1, 0,
            0, 0, 0, 0, 0,
            0, 0, 0, 0, 0
            });

    CapturePlan plan;
    bool ok;

    ok = map.run_test('M', plan);
    assert(ok);
    assert(plan.turns == 2);
    assert(plan.positions.size() == 6);
    assert(plan.last_wave.size() == 2);
    assert(!plan.last_wave_produces);
}

void testMaps() 
{
    std::unordered_map<Int2, int> m;
    m[Int2(0,0)] = 123;
    m[Int2(0,1)] = 456;
    assert( m[Int2(0,0)] == 123 );
    assert( m[Int2(0,1)] == 456 );

    std::unordered_map<Int2, std::set<Int2> > m2;
    Int2 u(12, 34);
    m2[u].insert( Int2(5,5) );
    m2[u].insert( Int2(6,6) );
    m2[u].insert( Int2(7,7) );
    assert( m2[u].size() == 3 );
    int sum = 0;
    for( Int2 v : m2[u] ) {
        sum += v.sum();
    }
    assert(sum == 10 + 12 + 14);

    m2[u].erase( Int2(5,5) );
    assert( m2[u].size() == 2 );
    sum = 0;
    for( Int2 v : m2[u] ) {
        sum += v.sum();
    }
    assert(sum == 12 + 14);
}

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

void testRange2()
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

void testIntRange() {
    int count = 0;
    for( int i : Range(5) ) {
        assert(i >= 0);
        assert(i < 5);
        count++;
    }

    assert(count == 5);

    count = 0;
    for( int i : Range(10,15) ) {
        assert(i >= 10);
        assert(i < 15);
        count++;
    }
    assert(count == 5);
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
    testIntRange();
    testRange2();
    testLambdaSort();
    testMaps();
    testCapture0();
    testCapture1();
    testCaptureWrap();
    testCaptureMultiChain();
    testCaptureMultiChain2();
}
