#include <cassert>

#include "../include/mds.hpp"

template <typename T>
mds<T>::mds() {}

template <typename T>
mds<T>::mds(std::vector<std::pair<T,T>> *I, T n, T a, int p, int v, bool log) {
    this->n = n;
    this->k = I->size();
    
    assert(0 < k && k <= n);
    assert(2 <= a);
    assert(1 <= p && p <= omp_get_max_threads() && (T) p <= n);
    assert((v == 1 && p == 1) || v == 2 || v == 3 || (v == 4 && 2 <= p));

    mdsb<T> mdsb(this,I,n,a,p,v,log);
}

template <typename T>
mds<T>::~mds() {}

template <typename T>
mds<T>::mds(std::istream &in) {
    in.read((char*)&n,sizeof(T));
    in.read((char*)&k,sizeof(T));

    D_pair.resize(k+1);
    in.read((char*)&D_pair[0],2*(k+1)*sizeof(T));

    D_index.resize(k);
    in.read((char*)&D_index[0],k*sizeof(T));
}

template <typename T>
uint64_t mds<T>::serialize(std::ostream &out) {
    out.write((char*)&n,sizeof(T));
    out.write((char*)&k,sizeof(T));
    out.write((char*)&D_pair[0],2*(k+1)*sizeof(T));
    out.write((char*)&D_index[0],k*sizeof(T));

    return (3*k+4)*sizeof(T);
}

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