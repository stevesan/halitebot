
#ifndef GRID2_HPP_INC
#define GRID2_HPP_INC

#include "Int2.hpp"
#include "Range2.hpp"
#include "Util.hpp"
#include <vector>

template <typename T> class Grid2 {
private:

    std::vector<T> data;
    Int2 dims;

public:

    Grid2( int width, std::vector<T> values )
    {
        reset(width, values);
    }

    Grid2( Int2 _dims )
    {
        resize(_dims);
    }

    Grid2()
    {
    }

    void resize(Int2 _dims) {
        dims = _dims;
        data.resize( dims.product() );
    }

    void set_all(T val) {
        for( Int2 u : indices() ) {
            set(u, val);
        }
    }

    void reset(int width, std::vector<T> values) {
        assert(values.size() % width == 0);
        int height = values.size() / width;
        dims = Int2(width, height);
        data = values;
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
    Int2 size() const { return dims; }

    Range yy() const { return Range(dims.y); }
    Range xx() const { return Range(dims.x); }

    Range2 indices() const {
        return Range2(Int2(0,0), dims);
    }
};


#endif
