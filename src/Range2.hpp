#ifndef RANGE2_HPP_INC
#define RANGE2_HPP_INC

#include <cmath>
#include <cassert>
#include "Int2.hpp"

// X-major iteration
class Range2Iter {

    Int2 mins, maxs;
    Int2 curr;

    public:

    Range2Iter( Int2 _mins, Int2 _maxs, Int2 _curr ) :
        mins(_mins), maxs(_maxs), curr(_curr)
    {
    }

    bool operator!=(Range2Iter other) const {
        return curr != other.curr;
    }

    Int2 operator*() const { return curr; }

    Range2Iter operator++() {
        curr.x++;
        if( curr.x >= maxs.x ) {
            curr.x = mins.x;
            curr.y++;
        }
        return *this;
    }
};

class Range2 {

    Int2 mins;
    Int2 maxs;

    public:

    Range2(Int2 _mins, Int2 _maxs) :
        mins(_mins), maxs(_maxs)
    {
        assert(mins.x <= maxs.x);
        assert(mins.y <= maxs.y);
    }

    Int2 getMins() { return mins; }
    Int2 getMaxs() { return maxs; }

    Range2Iter begin() {
        return Range2Iter(mins, maxs, mins);
    }

    Range2Iter end() {
        return Range2Iter(mins, maxs,
                // X-major
                Int2(mins.x, maxs.y));
    }
};

#endif
