#include <stdlib.h>
#include <time.h>
#include <cstdlib>
#include <ctime>
#include <time.h>
#include <set>
#include <fstream>

#include "hlt.hpp"
#include "networking.hpp"

typedef int utility;

std::string DIR2STR[] = { "Still", "North", "East", "South", "West" };

void output_moveset( std::ofstream& os, const hlt::MoveSet& moves ) {
    for(auto move = moves.begin(); move != moves.end(); ++move) {
        auto l = move->loc;
        auto d = move->dir;
        os << l.x << ", " << l.y << " : " << DIR2STR[d] << std::endl;
    }
}

int pickMove( const hlt::GameMap& map, hlt::Location u, hlt::PlayerId myId ) {
    hlt::Site curr = map.getSite(u);
    const int ENOUGH_STRENGTH = 200;

    hlt::Site dests[] = {
        curr,
        map.getSite(u, NORTH),
        map.getSite(u, EAST),
        map.getSite(u, SOUTH),
        map.getSite(u, WEST)};

    if( curr.strength < 4*curr.production ) {
        return STILL;
    }

    // if we can take any dests, do it
    for( int d : DIRECTIONS ) {
        auto nbor = dests[d];
        if( nbor.owner != myId && nbor.strength < curr.strength ) {
            return d;
        }
    }

    // if I am surrounded by at least 2 maxed out allies, move to one of the non-maxed out ones
    int numBigs = 0;
    for( int d : CARDINALS ) {
        auto nbor = dests[d];
        if( nbor.owner == myId && nbor.strength >= ENOUGH_STRENGTH ) {
            numBigs++;
        }
    }

    if( numBigs > 0 && numBigs < 4 ) {
        // ok, find a non-big and go there
        for( int d : CARDINALS ) {
            auto nbor = dests[d];
            if( nbor.owner == myId && nbor.strength < ENOUGH_STRENGTH ) {
                return d;
            }
        }
    }

    return STILL;
}

int main(int argc, char** argv) {
	srand(time(NULL));

    std::string dbgf("dbg.log");
    if( argc > 1 ) {
        dbgf = argv[1];
    }
	std::ofstream dbg(dbgf);

	std::cout.sync_with_stdio(0);

    hlt::PlayerId myId;
	hlt::GameMap presentMap;
	getInit(myId, presentMap);
	sendInit("TheDarkness");

    hlt::MoveSet moves;

    int frameCount = 0;

	while(true) {

        dbg << "-- frame " << frameCount << std::endl;
         
        getFrame(presentMap);
        moves.clear();

        FORMAP(presentMap, x, y) {
            hlt::Location u = {x,y};
            hlt::Site curr = presentMap.getSite(u);
            if( curr.owner != myId ) {
                continue;
            }
            moves.insert({ u, (unsigned char)pickMove(presentMap, u, myId) });
        }

        sendFrame(moves);
        frameCount++;
	}

	return 0;
}
