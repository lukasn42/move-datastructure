#pragma once

/**
 * @brief dynamically-growing insert-only no-copy datastructure
 * @tparam T value type
 */
template <typename T>
class dg_io_nc {
    protected:
    uint64_t size; // number of elements reserved in the datastructure
    std::vector<std::vector<T>> vectors; // vectors that store the elements

    public:
    /**
     * @brief creates an empty datastructure
     */
    dg_io_nc();

    /**
     * @brief creates an empty datastructure with a certain amount of elements reserved
     * @param size initially reserved number of elements
     */
    dg_io_nc(uint64_t size);

    /**
     * @brief deletes the datastructure
     */
    ~dg_io_nc();

    /**
     * @brief inserts an element into the datastructure, doubles the number of elements reserved
     *        reserved by the datastructure if it is full
     * @param v element
     * @return pointer to the element in the datastructure
     */
    T* emplace_back(T &v);

    /**
     * @brief inserts an element into the datastructure, doubles the number of elements reserved
     *        reserved by the datastructure if it is full
     * @param v element
     * @return pointer to the element in the datastructure
     */
    T* emplace_back(T &&v);
};