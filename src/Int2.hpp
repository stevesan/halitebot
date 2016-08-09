
#ifndef INT2_HPP_INC
#define INT2_HPP_INC

#include <cassert>
#include <functional>

static int safemod(int p, int q) {
    while(p < 0) p += q;
    return p % q;
}

class Int2 {
    public:

    int x, y;

    Int2() : x(0), y(0) {
    }

    Int2(int _x, int _y) : x(_x), y(_y) {
    }

    Int2(const Int2& src) : x(src.x), y(src.y) {
    }

    Int2 operator%(Int2 other) const {
        return Int2( safemod(x, other.x), safemod(y, other.y) );
    }

    Int2 operator+(Int2 other) const {
        return Int2( x + other.x, y + other.y );
    }

    Int2 operator-(Int2 other) const {
        return Int2( x - other.x, y - other.y );
    }

    Int2 operator=(Int2 other) {
        x = other.x;
        y = other.y;
        return *this;
    }

    bool operator==(Int2 other) const {
        return x == other.x && y == other.y;
    }

    bool operator!=(Int2 other) const {
        return !operator==(other);
    }

    // y first. To stay consistent with X-major range-iteration order.
    bool operator<(Int2 other) const {
        if( y > other.y ) {
            return false;
        }
        else if( y == other.y ) {
            return x < other.x;
        }
        else {
            return true;
        };
    }

    // NOTE NOTE NOTE: This is flipped Y...so NORTH actually results in y-1
    // Also..note that you should pass in NORTH-1 for NORTH, etc.
    // Yeh...really not happen with this.
    Int2 nbor(int d) const {
        assert( d >= 0 && d < 4 );
        static Int2 norms[4] = {Int2(0,-1), Int2(1,0), Int2(0,1), Int2(-1,0)};
        return (*this) + norms[d];
    }

    int product() const { return x * y; }
    int sum() const { return x + y; }

    template<typename Func>
    void forNbors(Func f) {
        for(int d = 0; d < 4; d++ ) {
            f(nbor(d));
        }
    }

    // Maxs is NOT inclusive
    template<typename Func>
    static void forRange(Int2 mins, Int2 maxs, Func f) {
        for( int x = mins.x; x < maxs.x; x++ ) {
            for( int y = mins.y; y < maxs.y; y++ ) {
                f(Int2(x,y));
            }
        }
    }

    int taxiDist(Int2 to) const {
        return std::abs(to.x - x) + std::abs(to.y - y);
    }
};

namespace std {
    template<> struct hash<Int2>
    {
        size_t operator()(const Int2& u) const
        {
            size_t hash = 23;
            hash = hash*31 + u.x;
            hash = hash*31 + u.y;
            return hash;
        }
    };
}

class NborsIter {
    int nborNum;
    Int2 u;

    public:

    NborsIter(Int2 _u, int _nborNum) :
        u(_u),
        nborNum(_nborNum)
    {
    }

    bool operator!=(NborsIter other) const {
        return nborNum != other.nborNum;
    }

    Int2 operator*() const { return u.nbor(nborNum); }

    NborsIter operator++() {
        nborNum++;
        return *this;
    }
};

// For iterating
class Nbors {
    Int2 u;

    public:

    Nbors(Int2 _u) : u(_u) {}

    NborsIter begin() {
        return NborsIter(u,0);
    }

    NborsIter end() {
        return NborsIter(u,4);
    }
};

#endif
