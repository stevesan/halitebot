
#ifndef UTIL_HPP_INC
#define UTIL_HPP_INC

#include <cassert>

template<typename E, typename K>
K minKey(const E* array, size_t count, std::function <K (E)> key)
{
    assert(count > 0);
    K best = key(array[0]);
    for( int i = 1; i < count; i++ ) {
        best = std::min(best, key(array[i]));
    }
    return best;
}

template<typename E, typename K>
K maxKey(const E* array, size_t count, std::function <K (E)> key)
{
    assert(count > 0);
    K best = key(array[0]);
    for( int i = 1; i < count; i++ ) {
        best = std::max(best, key(array[i]));
    }
    return best;
}

template<typename E, typename K>
int findMin(const E* array, size_t count, std::function <K (E)> key)
{
    assert(count > 0);
    int bestIdx = 0;
    K best = key(array[0]);
    for( int i = 1; i < count; i++ ) {
        int curr = key(array[i]);
        if( curr < best ) {
            bestIdx = i;
            best = curr;
        }
    }
    return bestIdx;
}

template<typename E, typename K>
int findMax(const E* array, size_t count, std::function <K (E)> key)
{
    assert(count > 0);
    int bestIdx = 0;
    K best = key(array[0]);
    for( int i = 1; i < count; i++ ) {
        int curr = key(array[i]);
        if( curr > best ) {
            bestIdx = i;
            best = curr;
        }
    }
    return bestIdx;
}

#endif
