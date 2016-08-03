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

typedef int utility;

std::string DIR2STR[] = { "Still", "North", "East", "South", "West" };

void output_moveset( std::ofstream& os, const hlt::MoveSet& moves ) {
    for(auto move = moves.begin(); move != moves.end(); ++move) {
        auto l = move->loc;
        auto d = move->dir;
        os << l.x << ", " << l.y << " : " << DIR2STR[d] << std::endl;
    }
}

// TEMP TEMP global var
hlt::Location startLoc;

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

    int pickMove(
            const hlt::GameMap& map,
            const hlt::GameMap& nextMap,
            hlt::Location u, hlt::PlayerId myId ) {
        hlt::Site curr = map.getSite(u);
        const int ENOUGH_STRENGTH = 255/2;

        hlt::Site dests[] = {
            curr,
            map.getSite(u, NORTH),
            map.getSite(u, EAST),
            map.getSite(u, SOUTH),
            map.getSite(u, WEST)};

        hlt::Site nextDests[] = {
            nextMap.getSite(u),
            nextMap.getSite(u, NORTH),
            nextMap.getSite(u, EAST),
            nextMap.getSite(u, SOUTH),
            nextMap.getSite(u, WEST)};

        if( curr.strength < 3*curr.production ) {
            return STILL;
        }

        // if we can take any dests, do it
        for( int d : CARDINALS ) {
            auto u = nextDests[d];
            if( u.owner != myId && u.strength < curr.strength ) {
                return d;
            }
        }

        // if I am surrounded by at least 2 maxed out allies, move to one of the non-maxed out ones
        int numBigs = 0;
        for( int d : CARDINALS ) {
            if( dests[d].owner == myId && dests[d].strength >= ENOUGH_STRENGTH ) {
                numBigs++;
            }
        }

        if( numBigs > 0 && numBigs < 4 ) {
            // overcrowded.
            // find the weakest one and merge, to minimize overflow
            int bestD = STILL;
            // ok, find a non-big and go there
            for( int d : CARDINALS ) {
                if( dests[d].owner == myId ) {
                    /*
                       if( bestD == STILL || dests[d].strength < dests[bestD].strength ) {
                       bestD = d;
                       }
                     */
                    if( dests[d].strength < ENOUGH_STRENGTH ) {
                        return d;
                    }
                }
            }

            return bestD;
        }

        return STILL;
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
        hlt::GameMap nextMap(presentMap);
        sendInit("TheDarkness");

        hlt::MoveSet moves;
        df.reset(mapDims());

        int frameCount = 0;
        std::queue<Int2> dfChanged;

        while(true) {
            getFrame(presentMap);
            presentMap.copyInto(nextMap);
            updateDF(dfChanged);
            moves.clear();

            FORMAP(presentMap, x, y) {
                hlt::Location u = {x,y};
                hlt::Site curr = presentMap.getSite(u);
                if( curr.owner != myId ) {
                    continue;
                }
                hlt::Move move = {u, (unsigned char)pickMove(presentMap, nextMap, u, myId)};
                nextMap.applyMove(move, myId);
                moves.insert(move);
            }

            sendFrame(moves);
            frameCount++;
        }

        return 0;
    }
};

int main(int argc, char** argv) {
    MyBot foo;
    return foo.main(argc, argv);
}
