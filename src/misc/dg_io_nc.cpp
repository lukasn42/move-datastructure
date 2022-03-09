#include <cstdint>

#include "../../include/misc/dg_io_nc.hpp"

template <typename T>
dg_io_nc<T>::dg_io_nc() {
    vectors = std::vector<std::vector<T>>(1,std::vector<T>());
    size = vectors.back().capacity();
}

template <typename T>
dg_io_nc<T>::dg_io_nc(uint64_t size) : dg_io_nc() {
    vectors.back().reserve(size);
    this->size = size;
}

template <typename T>
dg_io_nc<T>::~dg_io_nc() {
    size = 0;
}

template <typename T>
T* dg_io_nc<T>::emplace_back(T &&v) {
    if (vectors.back().size() == vectors.back().capacity()) {
        vectors.emplace_back(std::vector<T>());
        vectors.back().reserve(size);
        size *= 2;
    }

    vectors.back().emplace_back(v);

    return &(vectors.back().back());
}

template <typename T>
T* dg_io_nc<T>::emplace_back(T &v) {
    return emplace_back(std::move(v));
}