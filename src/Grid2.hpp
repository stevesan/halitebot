
#ifndef GRID2_HPP_INC
#define GRID2_HPP_INC

#include "Int2.hpp"

template <typename T> class Grid2 {
    public:

    T* data;
    Int2 dims;

    Grid2( Int2 _dims ) : data(NULL)
    {
        reset(_dims);
    }

    Grid2() : data(NULL)
    {
    }

    virtual ~Grid2() {
        delete data;
    }

    void reset(Int2 _dims) {
        dims = _dims;
        data = new T[dims.product()];
    }

    T get(Int2 u) const {
        u = u % dims;
        return data[u.y * width() + u.x];
    }

    void set(Int2 u, T val) {
        u = u % dims;
        data[u.y * width() + u.x] = val;
    }

    template<typename Func>
    void foreachValue(Func func) const {
        Int2::forRange(Int2(0,0), dims,
                [&] (Int2 u) {
                    func( this->get(u) );
                });
    }

    template<typename Func>
    void foreachPair(Func func) const {
        Int2::forRange(Int2(0,0), dims,
                [&] (Int2 u) {
                    func( u, this->get(u) );
                });
    }

    template<typename Func>
    void foreachKey(Func func) const {
        Int2::forRange(Int2(0,0), dims,
                [&] (Int2 u) {
                    func( u );
                });
    }

    T sum() {
        int s = 0;
        foreachValue([&](T val) { s += val; });
        return s;
    }

    void writeCsv( std::ostream& os ) {
        for( int y = 0; y < dims.y; y++ ) {
            for( int x = 0; x < dims.x; x++ ) {
                os << get(Int2(x,y)) << ",";
            }
            os << std::endl;
        }
    }

    int width() const { return dims.x; }
    int height() const { return dims.y; }
};


#endif
