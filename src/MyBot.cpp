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
#include <unordered_map>

#include "hlt.hpp"
#include "networking.hpp"

#include "Int2.hpp"
#include "Grid2.hpp"
#include "GridAlgos.hpp"
#include "Util.hpp"
#include "Capture.hpp"

typedef int utility;

std::string DIR2STR[] = { "Still", "North", "East", "South", "West" };

static std::ostream& operator<<( std::ostream& os, hlt::Location u ) {
    os << u.x << "," << u.y;
    return os;
}

class MyBot
{
    std::ofstream dbg;
    hlt::PlayerId myId;
	hlt::GameMap presentMap;
    Grid2<unsigned int> df;
    std::set<Int2> moved_cells;

    public:

    Int2 size() const {
        return Int2(presentMap.width, presentMap.height);
    }

    hlt::Location asLoc(Int2 u) const {
        u = u % size();
        return { (unsigned short)u.x, (unsigned short)u.y };
    }

    Int2 asInt2(hlt::Location u) {
        return Int2((int)u.x, (int)u.y);
    }

    Range2 cells() {
        return df.indices();
    }

    int getDF(Int2 u) { return df.get(u); }
    int getDF(Int2 u, int dir) {
        assert(dir >= 1 && dir <= 4);
        return df.get(u.nbor(dir-1)); }
    int getDF(hlt::Location u) { return getDF(asInt2(u)); }
    int getDF(hlt::Location u, int dir) { return getDF(asInt2(u), dir); }

    hlt::Site getSite(hlt::Location u, int dir) const { return presentMap.getSite(u,dir); }

    hlt::Site getSite(hlt::Location u) const { return presentMap.getSite(u); }

    hlt::Site getSite(Int2 u) const { return presentMap.getSite(asLoc(u)); }

    bool usable_for_capture(Int2 u) const {
        return getSite(u).owner == myId
            // if we moved a cell already this frame, we must've used it to capture something else
            && moved_cells.find(u) == moved_cells.end();
    }

    void updateDF(std::queue<Int2>& dfChanged)
    {
        const int DIST_MAX = size().sum();

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

    hlt::Move make_move(Int2 u, int d) {
        Int2 map_size = size();
        assert(u.x >= 0);
        assert(u.x < map_size.x);
        assert(u.y >= 0);
        assert(u.y < map_size.y);
        assert(d < 5);
        assert(d >= 0);
        return hlt::Move( {asLoc(u), (unsigned char)d} );
    }

    Int2 pick_first_target() {
        for( Int2 u : cells() ) {
            if( getSite(u).owner != myId ) {
                // adjacent to me?
                for( Int2 v : Nbors(u) ) {
                    v = v % size();
                    if( getSite(v).owner == myId ) {
                        return u;
                    }
                }
            }
        }
        return Int2(0,0);
    }

    float compute_target_util(Int2 u) {
        return getSite(u).production - 0.0*getSite(u).strength;
    }

    int main(int argc, char** argv) {
        srand(time(NULL));

        std::string dbgf("dbg.log");
        if( argc > 1 ) {
            dbgf = argv[1];
        }

        dbg.open(dbgf);

        std::cout.sync_with_stdio(0);

        getInit(myId, presentMap);
        sendInit("Roombo");

        hlt::MoveSet moves;
        df.resize(size());

        int frameCount = 0;
        std::queue<Int2> dfChanged;

        std::vector<Int2> my_cells;

        while(true) {
            dbg << "frame " << frameCount << std::endl;
            getFrame(presentMap);
            moves.clear();
            moved_cells.clear();

            //----------------------------------------
            //  Find targets
            //----------------------------------------
            std::set<Int2> targets;
            my_cells.clear();
            for(Int2 u : cells()) {
                if( getSite(u).owner != myId ) {
                    // on my border?
                    for( Int2 v : Nbors(u) ) {
                        if( getSite(v).owner == myId ) {
                            targets.insert(u);
                        }
                    }
                }
                else {
                    my_cells.push_back(u);
                }
            }

            if( targets.size() == 0 ) {
                sendFrame(moves);
                continue;
            }


            dbg << "found " << targets.size() << " on border, first: " << *targets.begin() << std::endl;

            //----------------------------------------
            //  Compute utils, allocate plans
            //----------------------------------------
            std::unordered_map<Int2, float> target_utils;
            std::unordered_map<Int2, CapturePlan> plans;
            for( Int2 t : targets ) {
                target_utils[t] = compute_target_util(t);
                plans[t] = CapturePlan();
            }

            //----------------------------------------
            //  Main greedy util-rate-capture loop
            //----------------------------------------
            std::unordered_map<Int2, std::set<Int2> > cell_targets;
            std::set<Int2> need_replan;
            std::set<Int2> can_capture;

            for( Int2 t : targets ) {
                need_replan.insert(t);
            }

            while(true) {
                //----------------------------------------
                //  Update plans that need updating
                //----------------------------------------
                for( Int2 t : need_replan ) {
                    CapturePlan& plan = plans[t];
                    if( compute_capture_plan(*this, t, plan) ) {
                        can_capture.insert(t);
                        for(Int2 u : plan.positions) {
                            cell_targets[u].insert(t);
                        }
                    }
                    else {
                        can_capture.erase(t);
                    }
                }
                need_replan.clear();

                if(can_capture.empty()) {
                    // all done
                    break;
                }

                //----------------------------------------
                //  Find and execute plan for capturable target with highest util rate
                //----------------------------------------
                auto util_rate = [&] (Int2 t) { return pow(target_utils[t],1) / pow(plans[t].turns,1); };
                Int2 best_target = *can_capture.begin();
                float best_rate = util_rate(best_target);
                for( Int2 t : can_capture ) {
                    if( util_rate(t) > best_rate ) {
                        best_rate = util_rate(t);
                        best_target = t;
                    }
                }

                dbg << "capturing target " << best_target << std::endl;
                output_moves( plans[best_target], best_target, size(), moves );

                // update some state
                can_capture.erase(best_target);
                for( Int2 u : plans[best_target].positions ) {
                    moved_cells.insert(u);
                }

                //----------------------------------------
                //  Mark the ones that need replan
                //  Go through all the cells we just used, and mark the targets that use them
                //----------------------------------------
                need_replan.clear();
                for(Int2 u : plans[best_target].positions ) {
                    for(Int2 t : cell_targets[u]) {
                        need_replan.insert(t);
                    }
                }
                // redundant
                need_replan.erase(best_target);
            }

            //----------------------------------------
            //  Now for all of mine that did not move..employ old logic
            //----------------------------------------
            assert(dfChanged.empty());
            collectBestTargets(dfChanged);
            updateDF(dfChanged);
            std::vector<Int2> old_move_cells;
            for( Int2 u : my_cells ) {
                if( moved_cells.find(u) != moved_cells.end() ) {
                    continue;
                }
                old_move_cells.push_back(u);
            }
            // move cells closer to edge first
            std::sort( old_move_cells.begin(), old_move_cells.end(),
                    [&] (Int2 a, Int2 b) {
                        return getDF(a) < getDF(b);
                    });
            for( Int2 u : old_move_cells ) {
                hlt::Location l = asLoc(u);
                hlt::Move move = {l, (unsigned char)pick_move(l)};
                apply_move(move);
                moves.insert(move);
            }

            //----------------------------------------
            //  Send moves
            //----------------------------------------
            dbg << "FINAL MOVES: " << std::endl;
            for(auto move : moves) {
                dbg << asInt2(move.loc) << " " << DIR2STR[move.dir] << std::endl;
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
