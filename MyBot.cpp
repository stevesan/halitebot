#include <stdlib.h>
#include <time.h>
#include <cstdlib>
#include <ctime>
#include <time.h>
#include <set>
#include <fstream>

#include "hlt.hpp"
#include "networking.hpp"

class Int2 {
    public:

    int x, y;

    Int2(int _x, int _y) : x(_x), y(_y)
    {
    }

    Int2(const Int2& src) : x(src.x), y(src.y)
    {
    }

    Int2 operator%(Int2 other) {
        return Int2( x % other.x, y % other.y );
    }

    Int2 operator+(Int2 other) {
        return Int2( x + other.x, y + other.y );
    }

    Int2 operator-(Int2 other) {
        return Int2( x - other.x, y - other.y );
    }

    int product() { return x * y; }
    int sum() { return x + y; }
};

template <typename T> class Grid2 {
    public:

    T* data;
    Int2 dims;

    Grid2( Int2 _dims ) : dims(_dims), data(NULL)
    {
        data = new T[dims.product()];
    }

    T get(Int2 u) {
        u = u % dims;
        return data[u.y * width() + u.x];
    }


    T set(Int2 u, T val) {
        u = u % dims;
        data[u.y * width() + u.x] = val;
    }

    int width() { return dims.x; }
    int height() { return dims.y; }
};

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

int pickMove( const hlt::GameMap& map, hlt::Location u, hlt::PlayerId myId ) {
    hlt::Site curr = map.getSite(u);
    const int ENOUGH_STRENGTH = 255/2;

    hlt::Site dests[] = {
        curr,
        map.getSite(u, NORTH),
        map.getSite(u, EAST),
        map.getSite(u, SOUTH),
        map.getSite(u, WEST)};

    if( curr.strength < 3*curr.production ) {
        return STILL;
    }

    // if we can take any dests, do it
    for( int d : CARDINALS ) {
        auto nbor = dests[d];
        if( nbor.owner != myId && nbor.strength < curr.strength ) {
            return d;
        }
    }

    // if we're surrounded by all friendlies...
    bool internal = true;
    for( int d : CARDINALS  ) {
        if( dests[d].owner != myId ) {
            internal = false;
            break;
        }
    }

    // move away from the cetner of mass if we're big enough and internal
    /*
    if( internal && curr.strength > ENOUGH_STRENGTH ) {
        int dx = 0;
        int dy = 0;
        int leftGoal = startLoc.x - map.width/2 + 1;
        int rightGoal = startLoc.y + map.width/2 - 1;
        if( abs(leftGoal - u.x) > abs(rightGoal - u.x) ) {
            // should go right
            dx = 1;
        }
        else {
            dx = 0;
        }

        int upGoal = startLoc.y + map.height/2 - 1;
        int downGoal = startLoc.y - map.height/2 + 1;
        if( abs(upGoal - u.y) > abs(downGoal - u.y) ) {
            // go down
            dy = 0;
        }
        else {
            dy = 1;
        }
    }
    */

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
            hlt::Move move = {u, (unsigned char)pickMove(presentMap, u, myId)};
            moves.insert(move);
            //presentMap.applyMove(move, myId);
        }

        sendFrame(moves);
        frameCount++;
	}

	return 0;
}
