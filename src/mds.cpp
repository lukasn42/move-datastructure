#include <cassert>

#include "../include/mds.hpp"

template <typename T>
mds<T>::mds(std::vector<std::pair<T,T>> *I, T n, T a, T b, int p, int v) {
    this->n = n;
    this->k = I->size();
    
    assert(0 < k && k <= n);
    assert(2 <= b && b < a-1);
    assert(1 <= p && p <= omp_get_max_threads() && (T) p <= k);
    assert((v == 1 && p == 1) || v == 2 || v == 3);

    mdsb<T> mdsb(this,I,n,a,b,p,v);
}

template <typename T>
mds<T>::~mds() {}

template <typename T>
T mds<T>::intervals() {
    return k;
}

template <typename T>
T mds<T>::max_value() {
    return n;
}

template <typename T>
std::pair<T,T>& mds<T>::pair(T i) {
    return D_pair[i];
}

template <typename T>
T mds<T>::index(T i) {
    return D_index[i];
}

template <typename T>
void mds<T>::move(std::pair<T,T> &ix) {
    ix.first = D_pair[ix.second].second+(ix.first-D_pair[ix.second].first);
    ix.second = D_index[ix.second];
    while (ix.first >= D_pair[ix.second+1].first) {
        ix.second++;
    }
}