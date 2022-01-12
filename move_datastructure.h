#pragma once

#include <iostream>
#include <cstdint>
#include <algorithm>
#include <fstream>
#include <string>
#include <vector>
#include <functional>

#include "avl_tree.h"
#include "avl_tree.cpp"

#include "dl_list.h"
#include "dl_list.cpp"

template <typename T> using interv_pair = std::pair<T,T>;
template <typename T> using interv_seq = std::vector<interv_pair<T>>;

template <typename T> using pair_list_node = dll_node<interv_pair<T>>;
template <typename T> using pair_list = dl_list<interv_pair<T>>;

template <typename T> using pair_tree_node = avl_node<pair_list_node<T>>;
template <typename T> using pair_tree = avl_tree<pair_list_node<T>>;

template <typename T>
class move_datastructure {
    protected:
    /* maximum value, n = p_{k-1} + d_{k-1}, k <= n */
    T n;

    /* number of intervals, 0 < k */
    T k;

    /* minimum number of incoming edges in the permutation graph, an output interval must have, to get cut, 4 <= a (default = 4) */
    uint8_t a;

    /* number of remaining incoming edges after an output interval gets cut, 2 <= b < a-1 (default = 2) */
    uint8_t b;

    /* balanced inteval sequence B_I */
    interv_seq<T> D_pair;

    /* D_index[j] = i <=> q_j in [p_i, p_i + d_i - 1], with i,j in [0..k-1] */
    std::vector<T> D_index;

    /**
     * @brief returns the length of an interval
     * 
     * @param nodePair pointer to the pair (p_i, q_i)
     * @return T length of the input/output interval starting at p_i/q_i
     */
    static inline T interval_length(pair_list_node<T>* nodePair);

    /**
     * @brief returns whether the an output interval is unbalanced
     * 
     * @param nodePairI pointer to the node in L_in, storing (p_i,q_i), [p_i, p_i + d_i - 1] must be the first input interval connected to [q_j, q_j + d_j - 1] in the permutation graph
     * @param nodePairJ pointer to the node in L_in, storing (p_j,q_j), [q_j, q_j + d_j - 1] must be the first unbalanced output interval
     * @return true if [q_j, q_j + d_j - 1] is unbalanced (has >= a incoming edges in the permutation graph)
     * @return false if [q_j, q_j + d_j - 1] is balanced(has < a incoming edges in the permutation graph)
     */
    inline bool is_unbalanced(pair_list_node<T>* nodePairI, pair_list_node<T>* nodePairJ);

    /**
     * @brief balances an output interval and all unbalanced output intervals starting before or at q_u, that have got unbalanced in the process
     * 
     * @param L_in pointer to L_in
     * @param T_out pointer to T_out
     * @param nodePairI pointer to the node in L_in, storing (p_i,q_i), [p_i, p_i + d_i - 1] must be the first input interval connected to [q_j, q_j + d_j - 1] in the permutation graph
     * @param nodePairJ pointer to the node in L_in, storing (p_j,q_j), [q_j, q_j + d_j - 1] must be the first unbalanced output interval
     * @param q_u starting position of an output interval
     */
    void balance_upto(pair_list<T> *L_in, pair_tree<T> *T_out, pair_list_node<T>* nodePairI, pair_list_node<T>* nodePairJ, T q_u);

    public:
    /**
     * @brief creates a move datastructure out of I by building L_in and T_out from I, balancing I and then building D_pair and D_index, in O(k log k)
     * 
     * @param I disjoint interval sequence I
     * @param n n = p_{k-1} + d_{k-1}, k <= n
     * @param a minimum number of incoming edges in the permutation graph, an output interval must have, to get cut, 4 <= a (default = 4)
     * @param b number of remaining incoming edges after an output interval is cut, 2 <= b < a-1 (default = 2)
     */
    move_datastructure(interv_seq<T> *I, T n, uint8_t a = 4, uint8_t b = 2);

    /**
     * @brief returns the number of intervals in D_pair
     * 
     * @return T k
     */
    T size();

    /**
     * @brief finds the input interval [p_x, p_x + d_x - 1], in which i lies, in O(log k)
     * 
     * @param i i in [0..n-1]
     * @return T x in [0..k-1], so that i in [p_x, p_x + d_x - 1]
     */
    T find_interval(T i);

    /**
     * @brief calculates the move query Move(I,i,x) = (i',x'), in O(a)
     * 
     * @param i i in [0..n-1]
     * @param x x in [0..k-1] and i in [p_x, p_x + d_x - 1]
     * @return interv_pair<T> (i',x'), with i' = f_I(i) and i' in [p_x', p_x' + d_x' - 1]
     */
    interv_pair<T> move(T i, T x);
};