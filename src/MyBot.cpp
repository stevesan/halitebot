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

void output_moveset( std::ofstream& os, const hlt::MoveSet& moves ) {
    for(auto move = moves.begin(); move != moves.end(); ++move) {
        auto l = move->loc;
        auto d = move->dir;
        os << l.x << ", " << l.y << " : " << DIR2STR[d] << std::endl;
    }
}

class MyBot
{
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

    void updateDF(std::queue<Int2>& dfChanged)
    {
        const int DIST_MAX = mapDims().sum();
        // init distance field
        assert( dfChanged.empty() );
        df.foreachKey([&](Int2 u) {
            if( presentMap.getSite(asLoc(u)).owner != myId ) {
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
        hlt::Site src = presentMap.getSite(u);

        if( src.strength < std::min(150, 3*src.production) ) {
            return STILL;
        }

        if( df.get(asInt2(u)) == 1 ) {
            auto this_attack_util = [&] (int d) -> int {
                assert (d != STILL);
                auto dst = presentMap.getSite(u, d);
                return attack_util(src, dst);
            };
            int d = CARDINALS[ findMax<int, int>(CARDINALS, 4, this_attack_util) ];

            // confirm this is a good idea at all..
            auto dest = presentMap.getSite(u,d);
            if( dest.owner == myId || dest.strength >= src.strength ) {
                // no good attack opp
                return STILL;
            }
            else {
                return d;
            }
        }
        else {
            auto this_move_cost = [&] (int d) -> int {
                assert (d != STILL);
                return move_cost(u, presentMap.getLocation(u,d));
            };
            int d = CARDINALS[ findMin<int, int>(CARDINALS, 4, this_move_cost) ];
            // only move if it gets use closer

            auto v = presentMap.getLocation(u,d);
            if( df.get(asInt2(v)) < df.get(asInt2(u)) ) {
                return d;
            }
            else {
                return STILL;
            }
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
        std::ofstream dbg(dbgf);

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
            dbg << "begin frame " << frameCount << std::endl;


            for( int phase_dist = 1; phase_dist <= maxDist; phase_dist++ ) {

                FORMAP(presentMap, x, y) {
                    hlt::Location u = {x,y};

                    if( df.get(asInt2(u)) != phase_dist ) {
                        continue;
                    }

                    hlt::Site curr = presentMap.getSite(u);
                    assert(curr.owner == myId);
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
        if( dst.owner == myId ) {
            return 0;
        }
        else if( dst.strength >= src.strength ) {
            return src.strength - dst.strength;
        }
        else {
            return dst.production;
        }
    }

    int move_cost(hlt::Location u, hlt::Location v) {
        auto src = presentMap.getSite(u);
        auto dst = presentMap.getSite(v);
        int opportunity_cost = src.production + dst.production;
        int overflow_cost = std::max(0, (src.strength + dst.strength) - 255);
        return opportunity_cost + overflow_cost;
    }
};

int main(int argc, char** argv) {
    MyBot foo;
    return foo.main(argc, argv);
}
