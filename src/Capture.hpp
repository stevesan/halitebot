
#ifndef CAPTURE_HPP_INC
#define CAPTURE_HPP_INC

#include "hlt.hpp"
#include "Int2.hpp"
#include <set>
#include <unordered_map>

class CapturePlan {
    public:
    int turns;
    std::set<Int2> positions;
    std::set<Int2> last_wave;
    bool last_wave_produces;

    void reset() {
        turns = 0;
        positions.clear();
        last_wave.clear();
        last_wave_produces = false;
    }
};

template< typename Map >
bool compute_capture_plan(const Map& map, Int2 target_pos, CapturePlan& plan )
{
    plan.turns = 0;
    plan.positions.clear();

    hlt::Site target = map.getSite(target_pos);

    std::vector<Int2> wave;
    std::set<Int2> visited;

    auto is_visited = [&] (Int2 u) { return visited.find(u % map.size()) != visited.end(); };
    auto str_gt = [&] (Int2 a, Int2 b) -> bool {
        return map.getSite(a).strength > map.getSite(b).strength;
    };

    auto str_prod_gt = [&] (Int2 a, Int2 b) -> bool {
        return (map.getSite(a).strength + map.getSite(a).production)
                >
                (map.getSite(b).strength + map.getSite(b).production);
    };

    auto advance_wave = [&] () {
        std::vector<Int2> prev_wave = wave;
        wave.clear();
        for( auto u : prev_wave ) {
            for( auto v : Nbors(u) ) {
                v = v % map.size();
                if( map.isOwned(v) && !is_visited(v) ) {
                    wave.push_back(v);
                    visited.insert(v);
                }
            }
        }
    };

    wave.push_back(target_pos);

    plan.reset();
    int total_str = 0;

    while(true) {
        advance_wave();
        plan.turns++;

        if(wave.size() == 0) {
            // impossible to capture!
            plan.turns = 0;
            plan.positions.clear();
            return false;
        }

        int prior_production = 0;
        // all positions of previous waves produced once
        for( Int2 u : plan.positions ) {
            prior_production += map.getSite(u).production;
        }
        total_str += prior_production;
        std::cout<< "total str " << total_str << std::endl;

        // can we get enough advancing this wave immediately?
        std::sort( wave.begin(), wave.end(), str_gt );
        bool immediate_move_enough = false;
        int wave_str = total_str;
        int last_pos = -1;
        for( int i = 0; i < wave.size(); i++ ) {
            Int2 u = wave[i];
            wave_str += map.getSite(u).strength;
            if( wave_str > target.strength ) {
                immediate_move_enough = true;
                last_pos = i;
                break;
            }
        }

        if(immediate_move_enough) {
            assert(last_pos >= 0 && last_pos < wave.size());
            for( int i = 0; i <= last_pos; i++ ) {
                plan.positions.insert(wave[i]);
                plan.last_wave.insert(wave[i]);
            }
            plan.last_wave_produces = false;
            return true;
        }

        // not enough, what if we produced one turn and then moved?
        std::sort( wave.begin(), wave.end(), str_prod_gt );
        bool produce_first_enough  = false;
        // we get another round of production from the existing positions
        wave_str = total_str + prior_production;
        last_pos = -1;
        for( int i = 0; i < wave.size(); i++ ) {
            Int2 u = wave[i];
            wave_str += map.getSite(u).strength + map.getSite(u).production;
            if( wave_str > target.strength ) {
                produce_first_enough  = true;
                last_pos = i;
                break;
            }
        }

        if(produce_first_enough ) {
            // that works - so we need 1 extra turn, but can stop before the next wave
            plan.turns++;
            assert(last_pos >= 0 && last_pos < wave.size());
            for( int i = 0; i <= last_pos; i++ ) {
                plan.positions.insert(wave[i]);
                plan.last_wave.insert(wave[i]);
            }
            plan.last_wave_produces = true;
            return true;
        }

        // ok, we'll need the help of the next wave
        // add us to the total strength
        for( Int2 u : wave ) {
            total_str += map.getSite(u).strength;
            plan.positions.insert(u);
        }
    }

    assert(false);
}

void output_moves( const CapturePlan& plan, Int2 target, Int2 map_size, hlt::MoveSet& moves ) {
    auto make_move = [&] (Int2 u, int d) {
        assert(u.x >= 0);
        assert(u.x < map_size.x);
        assert(u.y >= 0);
        assert(u.y < map_size.y);
        assert(d < 5);
        assert(d >= 0);
        hlt::Location loc( {(unsigned short)u.x, (unsigned short)u.y} );
        return hlt::Move( {loc, (unsigned char)d} );
    };

    for( Int2 u : plan.positions ) {
        if( plan.last_wave.find(u) != plan.last_wave.end() ) {
            // in last wave
            if( plan.last_wave_produces ) {
                moves.insert( make_move(u, STILL) );
            }
            else {
                // find the direction to move in.
                // there is only 1 non-last-wave position that is adjacent to this
                // so, find it
                bool found = false;
                for( unsigned char d : CARDINALS ) {
                    Int2 v = u.nbor(d-1) % map_size;
                    if( plan.positions.find(v) != plan.positions.end() ) {
                        moves.insert( make_move(u, d) );
                        found = true;
                    }
                    else if( v == target ) {
                        // only 1 wave, immediate move to target
                        assert( plan.positions.size() == plan.last_wave.size() );
                        moves.insert( make_move(u, d) );
                        found = true;
                    }
                }
                assert(found);
            }
        }
        else {
            // inner waves always produce
            moves.insert( make_move(u, STILL) );
        }
    }
}

#endif
