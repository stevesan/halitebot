#include <stdlib.h>
#include <time.h>
#include <cstdlib>
#include <ctime>
#include <time.h>
#include <set>
#include <fstream>
#include <queue>
#include <cassert>

#include "hlt.hpp"
#include "networking.hpp"

#include "Int2.hpp"
#include "Grid2.hpp"
#include "GridAlgos.hpp"
#include "Util.hpp"

typedef int utility;

std::string DIR2STR[] = { "Still", "North", "East", "South", "West" };

static std::ostream& operator<<( std::ostream& os, Int2 u ) {
    os << u.x << "," << u.y;
    return os;
}

static std::ostream& operator<<( std::ostream& os, hlt::Location u ) {
    os << u.x << "," << u.y;
    return os;
}

void output_moveset( std::ofstream& os, const hlt::MoveSet& moves ) {
    for(auto move = moves.begin(); move != moves.end(); ++move) {
        auto l = move->loc;
        auto d = move->dir;
        os << l.x << ", " << l.y << " : " << DIR2STR[d] << std::endl;
    }
}

class MyBot
{
    std::ofstream dbg;
    hlt::PlayerId myId;
	hlt::GameMap presentMap;
    Grid2<unsigned int> df;

    Int2 mapDims() {
        return Int2(presentMap.width, presentMap.height);
    }

    hlt::Location asLoc(Int2 u) {
        u = u % mapDims();
        return { (unsigned short)u.x, (unsigned short)u.y };
    }

    Int2 asInt2(hlt::Location u) {
        return Int2((int)u.x, (int)u.y);
    }

    int getDF(Int2 u) { return df.get(u); }
    int getDF(Int2 u, int dir) {
        assert(dir >= 1 && dir <= 4);
        return df.get(u.nbor(dir-1)); }
    int getDF(hlt::Location u) { return getDF(asInt2(u)); }
    int getDF(hlt::Location u, int dir) { return getDF(asInt2(u), dir); }

    hlt::Site getSite(hlt::Location u, int dir) { return presentMap.getSite(u,dir); }

    hlt::Site getSite(hlt::Location u) { return presentMap.getSite(u); }

    hlt::Site getSite(Int2 u) { return presentMap.getSite(asLoc(u)); }

    void updateDF(std::queue<Int2>& dfChanged)
    {
        const int DIST_MAX = mapDims().sum();
        // init distance field
        assert( dfChanged.empty() );
        df.foreachKey([&](Int2 u) {
            if( getSite(u).owner != myId ) {
                df.set(u, 0);
                dfChanged.push(u);
            }
            else {
                df.set(u, DIST_MAX);
            }
        });
        updateDistanceField(df, dfChanged);
        assert( dfChanged.empty() );
    }

    int pick_move( hlt::Location u )
    {
        hlt::Site src = getSite(u);

        if( src.strength < std::min(200, 3*src.production) ) {
            return STILL;
        }

        if( getDF(u) == 1 ) {
            auto this_attack_util = [&] (int d) -> int {
                return attack_util(src, getSite(u,d));
            };
            int d = CARDINALS[ findMax<int, int>(CARDINALS, 4, this_attack_util) ];

            // it's actually possible that during the course of the frame,
            // all the foreign tiles we're adjacent to got defeated. so, if this is the case,
            // don't do anything.
            // THIS STRENGTH CHECK IS REALLY IMPORTANT
            if(getSite(u,d).owner == myId || getSite(u,d).strength > src.strength) {
                return STILL;
            }
            else {
                return d;
            }
        }
        else {
            //dbg << "begin findmin, u=" << u << std::endl;
            auto this_move_cost = [&] (int d) -> int {
                //dbg << u << " -"<<d<<"> " << asInt2(u).nbor(d-1) << " df=" << getDF(u,d) << std::endl;
                int cost = move_cost(u, presentMap.getLocation(u,d));
                //dbg << "cost = " << cost << std::endl;
                assert (cost >= 0);
                return cost;
            };
            int d = CARDINALS[ findMin<int, int>(CARDINALS, 4, this_move_cost) ];

            // we should be guaranteed to move closer to border
            //dbg << getDF(u,d) << " " << getDF(u) << std::endl;
            assert( getDF(u,d) < getDF(u) );
            return d;
        }

    }

    void apply_move(hlt::Move move) {
        auto src = presentMap.getSite(move.loc);
        auto dst = presentMap.getSite(move.loc, move.dir);

        if(move.dir == STILL) {
            src.strength += src.production;
        }
        else {
            if( dst.owner != src.owner ) {
                dst.strength -= src.strength;
                if( dst.strength <= 0) {
                    dst.strength = 0;
                    dst.owner = myId;
                }
            }
            else {
                dst.strength = std::min(255, (int)src.strength + (int)dst.strength);
            }

            src.strength = 0;
        }

        presentMap.setSite(move.loc, src);
        presentMap.setSite(move.loc, move.dir, dst);
    }

    public:

    int main(int argc, char** argv) {
        srand(time(NULL));

        std::string dbgf("dbg.log");
        if( argc > 1 ) {
            dbgf = argv[1];
        }

        dbg.open(dbgf);

        std::cout.sync_with_stdio(0);

        getInit(myId, presentMap);
        sendInit("TheDarkness");

        hlt::MoveSet moves;
        df.reset(mapDims());

        int frameCount = 0;
        std::queue<Int2> dfChanged;

        while(true) {
            getFrame(presentMap);
            updateDF(dfChanged);
            assert(dfChanged.empty());
            moves.clear();

            // find the max distance
            int maxDist = 0;
            df.foreachValue([&](int d) {
                    maxDist = std::max(d, maxDist);
                    });

            for( int phase_dist = 1; phase_dist <= maxDist; phase_dist++ ) {

                FORMAP(presentMap, x, y) {
                    hlt::Location u = {x,y};

                    if( getDF(u) != phase_dist ) {
                        continue;
                    }
                    assert(getSite(u).owner == myId);

                    hlt::Move move = {u, (unsigned char)pick_move(u)};
                    apply_move(move);
                    moves.insert(move);
                }
            }

            sendFrame(moves);
            frameCount++;
        }

        return 0;
    }

    int attack_util(hlt::Site src, hlt::Site dst) {
        if( dst.owner == myId || dst.strength >= src.strength ) {
        //if( dst.owner == myId ) {
            return -9999;
        }
        else if( dst.strength >= src.strength ) {
            return src.strength - dst.strength;
        }
        else {
            return dst.production;
        }
    }

    int move_cost(hlt::Location u, hlt::Location v) {
        // always prefer the ones closer to the border
        // so assign very high cost to any distance away from border
        //dbg << "v=" << v << " df=" << getDF(v) << std::endl;
        int border_cost = getDF(v) * 1000;

        auto src = presentMap.getSite(u);
        auto dst = presentMap.getSite(v);
        int opportunity_cost = src.production + dst.production;
        int overflow_cost = std::max(0, (src.strength + dst.strength) - 255);

        assert (opportunity_cost >= 0);
        assert (overflow_cost >= 0);
        assert (border_cost >= 0);

        return border_cost + opportunity_cost + overflow_cost;
    }
};

int main(int argc, char** argv) {
    // sanity tests
    Int2 u(1,1);
    assert(u.nbor(NORTH-1) == Int2(1,0));
    assert(u.nbor(SOUTH-1) == Int2(1,2));
    assert(u.nbor(EAST-1) == Int2(2,1));
    assert(u.nbor(WEST-1) == Int2(0,1));

    MyBot foo;
    return foo.main(argc, argv);
}
