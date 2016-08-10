#include <stdlib.h>
#include <time.h>
#include <cstdlib>
#include <ctime>
#include <time.h>
#include <set>
#include <fstream>
#include <queue>
#include <cassert>
#include <algorithm>

#include "hlt.hpp"
#include "networking.hpp"

#include "Int2.hpp"
#include "Grid2.hpp"
#include "GridAlgos.hpp"
#include "Util.hpp"

typedef int utility;

std::string DIR2STR[] = { "Still", "North", "East", "South", "West" };

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

    Range2 mapRange() {
        return df.indices();
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

        for(Int2 u : df.indices()) {
            df.set(u, DIST_MAX);
        }

        // a bit silly pattern..but..whatever
        for( int i = 0; i < dfChanged.size(); i++ ) {
            Int2 u = dfChanged.front();
            dfChanged.pop();
            df.set(u, 0);
            dfChanged.push(u);
        }

        updateDistanceField(df, dfChanged);
        assert( dfChanged.empty() );
    }

    void collectBestTargets(std::queue<Int2>& out) {
        // set of all non-mine positions that are adjacent to one of mine
        std::vector<Int2> targets;
        for(Int2 u : df.indices()) {
            if( getSite(u).owner != myId ) {
                // adjacent to me?
                for( Int2 v : Nbors(u) ) {
                    if( getSite(v).owner == myId ) {
                        // yes, add this!
                        targets.push_back(u);
                        break;
                    }
                }
            }
        }

        // TEMP TEMP
        // ensure all unique
        /*
        {
            std::set<Int2> targetsSet;
            for( Int2 u : targets ) {
                assert( targetsSet.find(u) == targetsSet.end() );
                targetsSet.insert(u);
            }
        }
        */

        // now sort by our utility and add the top x-percent to out
        const float STR_PENALTY = 0.1;
        auto util = [&] (Int2 u) {
            return getSite(u).production - getSite(u).strength*STR_PENALTY; };
        auto util_lt = [&] (Int2 u, Int2 v) -> bool { return util(u) < util(v); };
        std::sort(targets.begin(), targets.end(), util_lt);

        // now sorted in ascending order - grab the top half
        // 2/3 was bad here..
        // 1/3 was about as good as 1/2
        //int first = targets.size() * 2 / 3;
        int first = targets.size() * 2 / 3;
        for( int i = first; i < targets.size(); i++ ) {
            out.push(targets[i]);
        }
    }

    int pick_move( hlt::Location u )
    {
        hlt::Site src = getSite(u);

        if( src.strength < std::min(200, 3*src.production) ) {
            return STILL;
        }

        if( getDF(u) == 1 ) {
            auto attack_util = [&] (int d) -> int {
                auto dst = getSite(u,d);
                auto src = getSite(u);

                if( d == STILL ) {
                    // make this default
                    return 0;
                }
                if( getDF(u,d) != 0 ) {
                    // not a high priority target
                    return -9999;
                }
                if( dst.owner == myId ) {
                    return -9999;
                }
                // THIS STRENGTH CHECK IS REALLY IMPORTANT
                if(dst.strength >= src.strength ) {
                    return -9999;
                }

                assert (dst.production >= 0);
                // add 1 to put this above STILL
                return dst.production + 1;
            };
            int d = DIRECTIONS[ findMax<int, int>(DIRECTIONS, 5, attack_util) ];
            return d;
        }
        else {
            auto this_move_cost = [&] (int d) -> int {
                if( d == STILL ) {
                    return 100000;
                }
                int cost = move_cost(u, presentMap.getLocation(u,d));
                assert (cost >= 0);
                return cost;
            };
            int d = DIRECTIONS[ findMin<int, int>(DIRECTIONS, 5, this_move_cost) ];

            //assert( getDF(u,d) < getDF(u) );
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
        df.resize(mapDims());

        int frameCount = 0;
        std::queue<Int2> dfChanged;

        while(true) {
            getFrame(presentMap);

            assert(dfChanged.empty());
            collectBestTargets(dfChanged);
            updateDF(dfChanged);

            moves.clear();

            // find the max distance
            unsigned int maxDist = 0;
            for( Int2 u : df.indices() ) {
                if( getSite(u).owner == myId ) {
                    maxDist = std::max( df.get(u), maxDist );
                }
            }

            for( int phase_dist = 1; phase_dist <= maxDist; phase_dist++ ) {

                FORMAP(presentMap, x, y) {
                    hlt::Location u = {x,y};

                    if( getDF(u) != phase_dist ) {
                        continue;
                    }

                    if( getSite(u).owner != myId ) {
                        continue;
                    }

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

    int move_cost(hlt::Location u, hlt::Location v) {
        if( getSite(v).owner != myId ) {
            // do not move to non-owned - that is related to attacking!
            return 1000000;
        }
        // always prefer the ones closer to the border
        // so assign very high cost to any distance away from border
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
