
#ifdef CAPTURE_HPP_INC
#define CAPTURE_HPP_INC

#include "hlt.hpp"
#include "Int2.hpp"
#include <set>
#include <unordered_map>

struct CapturePlan {
    int turns;
    std::set<Int2> positions;
};

template< typename Map >
bool compute_capture_plan(const Map& map, Int2 target_pos, CapturePlan& out )
{
    out.turns = 0;
    out.used_squares.clear();

    hlt::Site target = map.getSite(target_pos);

    std::vector<Int2> wave;
    std::set<Int2> visited;

    auto is_visited = [&] (Int2 u) { return visited.find(u) != visited.end(); };
    auto str_lessthan = [&] (Int2 a, Int2 b) -> bool {
        return map.getSite(a).strength < map.getSite(b).strength;
    };

    auto advance_wave = [&] () {
        std::vector<Int2> prev_wave = wave;
        wave.clear();
        for( auto u : prev_wave ) {
            for( auto v : Nbors(u) ) {
                if( map.isOwned(v) && !is_visited(v) ) {
                    wave.push_back(v);
                }
            }
        }
        std::sort( wave.begin(), wave.end(), str_lessthan );
    };

    wave.push_back(target_pos);

    out.turns = 0;
    out.positions.clear();
    int total_str = 0;

    while(true) {
        advance_wave();
        out.turns++;

        if(wave.size() == 0) {
            // impossible!
            out.turns = 0;
            out.positions.clear();
            return false;
        }

        // all positions of previous waves produced once
        for( Int2 u : out.positions ) {
            total_str += getSite(u).production;
        }

        // can we get enough advancing this wave immediately?
        int wave_str = total_str;
        bool wave_enough = false;
        int last_pos = -1;
        for( int i = 0; i < wave.size(); i++ ) {
            wave_str += getSite(u).strength;
            if( wave_str > target.strength ) {
                wave_enough = true;
                last_pos = i;
                break;
            }
        }

        if(wave_enough) {
            assert(last_pos >= 0 && last_pos < wave.size();
            for( int i = 0; i <= last_pos; i++ ) {
                out.positions.push_back(wave[i]);
            }
            return true;
        }

        // not enough, what if we produced one turn and then moved?
        wave_enough = false;
        wave_str = total_str;
        last_pos = -1;
        for( int i = 0; i < wave.size(); i++ ) {
            wave_str += getSite(u).strength + getSite(u).production;
            if( wave_str > target.strength ) {
                wave_enough = true;
                last_pos = i;
                break;
            }
        }

        if(wave_enough) {
            // that works - so we need 1 extra turn, but can stop before the next wave
            out.turns++;
            assert(last_pos >= 0 && last_pos < wave.size();
            for( int i = 0; i <= last_pos; i++ ) {
                out.positions.push_back(wave[i]);
            }
            return true;
        }

        // ok, we'll need the help of the next wave
        // add us to the total strength
        for( Int u : wave ) {
            total_str += getSite(u).strength;
        }
    }
}

#endif
