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

template<typename MAP> utility evalUtil(unsigned char myId, const MAP& map)
{
    // first, count how many i have vs. other players (non-0)
    unsigned int player2terr[MAX_PLAYERS];
    memset(player2terr, 0, sizeof(player2terr[0]) * MAX_PLAYERS);

    FORMAP(map, x, y) {
        hlt::Site site = map.getSite({x,y});
        if( site.owner != 0 ) {
            player2terr[site.owner]++;
        }
    }

    unsigned int bestPlayer = -1;
    for( int i = 1; i < MAX_PLAYERS; i++ ) {
        if( bestPlayer == -1 || player2terr[i] > player2terr[bestPlayer] ) {
            bestPlayer = i;
        }
    }

    bool catchUp = false;
    const int lagTiles = 2;
    if( bestPlayer != myId && player2terr[bestPlayer] > player2terr[myId] + lagTiles ) {
        catchUp = true;
    }


    int terrUtil = catchUp ? 256 : 20;
    utility util = 0;
    FORMAP(map, x, y) {
        hlt::Site site = map.getSite({x,y});
        if( site.owner == myId ) {
            util += terrUtil + site.strength;
        }
    }
    return util;
}

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

    hlt::Site dests[] = {
        curr,
        map.getSite(u, NORTH),
        map.getSite(u, EAST),
        map.getSite(u, SOUTH),
        map.getSite(u, WEST)};

    if( curr.strength < 5*curr.production ) {
        return STILL;
    }

    // if we can take any dests, and the production is higher, do it with some prob
    for( int d : DIRECTIONS ) {
        auto nbor = dests[d];
        if( nbor.owner != myId &&  nbor.strength < curr.strength ) {
            return d;
        }
    }
    return STILL;
}

int main() {
	srand(time(NULL));

	std::ofstream dbg("dbg.log");

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
