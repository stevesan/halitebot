
#ifndef INT2_HPP_INC
#define INT2_HPP_INC

class Int2 {
    public:

    int x, y;

    Int2() : x(0), y(0) {
    }

    Int2(int _x, int _y) : x(_x), y(_y) {
    }

    Int2(const Int2& src) : x(src.x), y(src.y) {
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

    Int2 nbor(int d) {
        static Int2 norms[4] = {Int2(0,1), Int2(1,0), Int2(0,-1), Int2(-1,0)};
        return (*this) + norms[d];
    }

    int product() { return x * y; }
    int sum() { return x + y; }

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

};

#endif
