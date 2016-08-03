
#ifndef GRID_ALGOS_HPP_INC
#define GRID_ALGOS_HPP_INC

#include <queue>
#include "Grid2.hpp"

template<typename T> void updateDistanceField( Grid2<T>& df, std::queue<Int2>& changed )
{
    while(!changed.empty()) {
        Int2 u = changed.front();
        changed.pop();

        T myDist = df.get(u);
        u.forNbors([&](Int2 v) {
            if( df.get(v) > myDist + 1 ) {
                df.set(v, myDist + 1);
                changed.push(v);
            }
        });
    }
}


#endif
