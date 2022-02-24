#pragma once

#include <cstdint>
#include <vector>

#include "mdsb/mdsb.hpp"
#include "../src/mdsb/mdsb.cpp"

/**
 * @brief stores a bijective function f_I : [0..n-1] -> [0..n-1] as a balanced disjoint interval sequence B_I[0..k]
 *        (in the array D_pair), supports calculation of f_I(i) = i', with i in [0..n-1], by calculating Move(i,x) = (i',x')
 * @tparam T (integer) type of the interval starting positions
 */
template <typename T>
class mds {
    friend class mdsb<T>;

    protected:
    T n; // maximum value, n = p_{k-1} + d_{k-1}, k <= n
    T k; // number of intervals in the balanced disjoint inteval sequence B_I, 0 < k
    std::vector<std::pair<T,T>> D_pair; // stores the balanced disjoint inteval sequence B_I = ((p_0,q_0),(p_1,q_1),..,(p_{k-1},q_{k-1}))
    std::vector<T> D_index; // D_index[j] = i <=> q_j in [p_i, p_i + d_i - 1], with i,j in [0..k-1]

    public:
    /**
     * @brief creates an empty move datastructure
     */
    mds<T>();

    /**
     * @brief creates a move datastructure out of I by building L_in and T_out from I, balancing I and then building D_pair and D_index
     * @param I disjoint interval sequence I
     * @param n n = p_{k-1} + d_{k-1}, k <= n
     * @param a (optional) balancing parameter, balancing parameter, restricts size increase
     *          to the factor (1+1/(a-1)) and restricts move query runtime to 2a, 2 <= a
     * @param p (optional) number of threads to use (default: all threads)
     * @param v version of the build method (1/2/3/4) (default: 3)
     * @param log enables log messages during build process (default: false)
     */
    mds<T>(std::vector<std::pair<T,T>> *I, T n, T a = 2, int p = omp_get_max_threads(), int v = 3, bool log = false);

    /**
     * @brief deletes the move datastructure
     */
    ~mds<T>();

    /**
     * @brief creates a move datastructure from an input stream
     * @param in input stream
     */
    mds<T>(std::istream &in);

    /**
     * @brief writes the move_datastructure to ab output stream
     * @param out output stream
     * @return size of the data written to out
     */
    uint64_t serialize(std::ostream &out);

    /**
     * @brief returns the number of intervals in the disjoint interval sequnece (k)
     * @return number of intervals in the disjoint interval sequnece (k)
     */
    T intervals();

    /**
     * @brief returns n
     * @return n = p_{k-1} + d_{k-1}
     */
    T max_value();

    /**
     * @brief returns D_pair[i]
     * @param i in [0..k]
     * @return D_pair[i]
     */
    std::pair<T,T>& pair(T i);

    /**
     * @brief returns D_index[i]
     * @param i in [0..k-1]
     * @return D_index[i]
     */
    T index(T i);

    /**
     * @brief calculates the move query Move(I,i,x) = (i',x') by changing ix = (i,x)
     *        to ix' = (i',x'), with i' = f_I(i) and i' in [p_x', p_x' + d_x' - 1]
     * @param ix (i,x), i in [0..n-1], x in [0..k-1] and i in [p_x, p_x + d_x - 1]
     */
    void move(std::pair<T,T> &ix);
};