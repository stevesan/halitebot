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

int main() {
	srand(time(NULL));

	std::ofstream dbg("dbg.log");

	std::cout.sync_with_stdio(0);

    hlt::PlayerId myId;
	hlt::GameMap presentMap;
	getInit(myId, presentMap);
	sendInit("TheDarkness");

    unsigned int evalsPerFrame = 100;
    std::vector< hlt::MoveSet > movesets(evalsPerFrame);
	hlt::GameMap workMap(presentMap);

    int frameCount = 0;

	while(true) {

        dbg << "-- frame " << frameCount << std::endl;
         
        getFrame(presentMap);
        int bestset = -1;
        utility bestutil = 0;

        for( int i = 0; i < evalsPerFrame; i++ ) {
            hlt::MoveSet& moves = movesets[i];
            moves.clear();

            FORMAP(presentMap, x, y) {
                hlt::Location u = {x,y};
                hlt::Site src = presentMap.getSite(u);
                if (src.owner == myId) {
                    unsigned char d = (unsigned char)(rand() % 5);
                    moves.insert({ u, d });
                }
            }

            presentMap.copyInto(workMap);
            workMap.simMoves(moves, myId);
            utility util = evalUtil(myId, workMap);

            if(util > bestutil || bestset == -1) {
                bestutil = util;
                bestset = i;
            }
        }

        dbg << "using moveset " << bestset << " with util " << bestutil << std::endl;
        output_moveset(dbg, movesets[bestset]);
        sendFrame(movesets[bestset]);
        frameCount++;
	}

	return 0;
}
