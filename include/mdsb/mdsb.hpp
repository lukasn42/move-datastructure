#pragma once

#include <mutex>
#include <queue>

#include "../misc/avl_tree.hpp"
#include "../../src/misc/avl_tree.cpp"

#include "../misc/dl_list.hpp"
#include "../../src/misc/dl_list.cpp"

#include "../../extern/concurrentqueue/concurrentqueue.h"

template <typename T> class mds;

template <typename T> using interv_pair = std::pair<T,T>;
template <typename T> using interv_seq = std::vector<interv_pair<T>>;

// ############################# V2/3/4 #############################

template <typename T> using pair_list_node = dll_node<interv_pair<T>>;
template <typename T> using pair_list = dl_list<interv_pair<T>>;

template <typename T> using pair_tree_node = avl_node<pair_list_node<T>>;
template <typename T> using pair_tree = avl_tree<pair_list_node<T>>;

// ############################# V2 SEQUENTIAL #############################

template <typename T> using te_pair_seq = std::pair<pair_list_node<T>*,pair_tree_node<T>*>;
template <typename T> using te_node_seq = avl_node<te_pair_seq<T>>;
template <typename T> using te_tree_seq = avl_tree<te_pair_seq<T>>;

// ############################# V2 PARALLEL #############################

template <typename T> using te_pair_par = std::tuple<pair_list_node<T>*,pair_tree_node<T>*,pair_tree_node<T>*>;
template <typename T> using te_node_par = avl_node<te_pair_par<T>>;
template <typename T> using te_tree_par = avl_tree<te_pair_par<T>>;

// ############################# V3/V4 PARALLEL #############################

template <typename T> using ins_pair = std::pair<pair_list_node<T>*,pair_list_node<T>*>;
template <typename T> using ins_matr = std::vector<std::vector<std::queue<ins_pair<T>>>>;

// ############################# V4 PARALLEL #############################

template <typename T> using ins_matr_conc = std::vector<std::vector<moodycamel::ConcurrentQueue<ins_pair<T>>>>;

/**
 * @brief builds a mds
 * @tparam T (integer) type of the interval starting positions
 */
template <typename T>
class mdsb {
    public:
    /**
     * @brief builds the move datastructure mds
     * @param mds a move datastructure that has not yet been built
     * @param I disjoint interval sequence
     * @param n n = p_{k-1} + d_{k-1}, k <= n
     * @param a balancing parameter, restricts size increase to the factor
     *          (1+1/(a-1)) and restricts move query runtime to 2a, 2 <= a
     * @param p number of threads to use
     * @param v version of the build method (1/2/3/4)
     * @param log enables log messages during build process
     */
    mdsb<T>(mds<T> *mds, interv_seq<T> *I, T n, T a, int p, int v, bool log);

    /**
     * @brief deletes the mdsb
     */
    ~mdsb<T>();

    // ############################# VARIABLES #############################

    protected:
    mds<T> *md; // pointer to the move datastructure to build
    T n; // maximum value, n = p_{k-1} + d_{k-1}, k <= n
    T k; // number of intervals in the balanced inteval sequence B_I, 0 < k
    T a; // balancing parameter, restricts size increase to the factor (1+1/(a-1)), 2 <= a
    int p; // number of threads to use

    // ############################# V1 #############################

    /**
     * @brief builds the move datastructure md
     * @param I disjoint interval sequence
     * @param log enables log messages
     */
    void build_v1(interv_seq<T> *I, bool log);

    // ############################# V2/V3 #############################

    /** 
     * @brief [0..p-1] doubly linked lists; L_in[i_p] stores the pairs (p_i,q_i) in ascending order of p_i,
     *        where s[i_p] <= p_i < s[i_p+1] and i_p in [0..p-1]. L_in[0] ++ L_in[1] ++ ... ++ L_in[p-1] = I.
     */
    std::vector<pair_list<T>> L_in;
    /**
     * @brief [0..p-1] avl trees; T_out[i_p] stores nodes of lists in L_in in ascending order of q_i,
     *        for each pair (p_i,q_i), s[i_p] <= q_i < s[i_p+1] holds, with i_p in [0..p-1].
     */
    std::vector<pair_tree<T>> T_out;
    /**
     * @brief [0..p-1] section start positions in the range [0..n], 0 = s[0] < s[1] < ... < s[p-1] = n.
     *        Before building T_out, s is chosen so that |L_in[0]| ~ |L_in[1]| ~ ... ~ |L_in[p-1]| (see descr. of L_in).
     */
    std::vector<T> s;

    /**
     * @brief builds the move datastructure md
     * @param I disjoint interval sequence
     * @param v version of the build method
     * @param log enables log messages
     */
    void build_v2_v3_v4(interv_seq<T> *I, int v, bool log);

    // ############################# V2/V3/V3 SEQUENTIAL/PARALLEL #############################

    /**
     * @brief builds L_in[0..p-1] and T_out[0..p-1] out of the disjoint interval sequence I
     * @param I disjoint interval sequence
     */
    void build_lin_tout(interv_seq<T> *I);

    /**
     * @brief inserts the pairs in L_in[0..p-1] into D_pair
     */
    void build_dpair();

    /**
     * @brief deletes L_in and T_out
     */
    void delete_lin_tout();

    /**
     * @brief builds D_index
     */
    void build_dindex();

    /** 
     * @brief returns the length of the input/output interval starting at p_i/q_i
     * @param pln (p_i,q_i)
     * @return length of the input/output interval starting at p_i/q_i
     */
    static T interval_length_seq(pair_list_node<T>* pln);

    /**
     * @brief checks if [q_j, q_j + d_j - 1] is unbalanced and returns the a+1-th input interval starting in it
     * @param pln_I (p_i,q_i), [p_i, p_i + d_i - 1] must be the first input interval connected to [q_j, q_j + d_j - 1] in the permutation graph
     * @param ptn_J (p_j,q_j)
     * @return (p_{i+a},q_{i+a}) if [q_j, q_j + d_j - 1] is unbalanced, else NULL
     */
    inline pair_list_node<T>* is_unbalanced_seq(pair_list_node<T>* pln_I, pair_tree_node<T>* ptn_J);

    // ############################# V2 SEQUENTIAL #############################

    /**
     * @brief balances the disjoint interval sequence in L_in[0] and T_out[0]
     */
    void balance_v2_seq();

    // ############################# V2 PARALLEL #############################

    /**
     * @brief reduces T_e by inserting pairs into L_in[i_p] and Q_ins[0..p-1][i_p]
     * @param T_e reference to T_e
     * @param Q_ins reference to Q_ins
     */
    void reduce_te(te_tree_par<T> &T_e, ins_matr<T> &Q_ins);

    /**
     * @brief balances the disjoint interval sequence in L_in[0..p-1] and T_out[0..p-1]
     */
    void balance_v2_par();

    // ############################# V3 SEQUENTIAL #############################

    /**
     * @brief balances the output interval [q_j, q_j + d_j - 1] and all unbalanced output intervals
     *        starting before or at q_u that have become unbalanced in the process
     * @param pln_IpA (p_{i+a},q_{i+a}), [p_i, p_i + d_i - 1] must be the first input interval connected to [q_j, q_j + d_j - 1] in the permutation graph
     * @param ptn_J (p_j,q_j), [q_j, q_j + d_j - 1] must be the first unbalanced output interval
     * @param q_u starting position of an output interval
     * @return the newly created pair (p_j+d,q_j+d)
     */
    inline pair_tree_node<T>* balance_upto_seq(pair_list_node<T>* pln_IpA, pair_tree_node<T>* ptn_J, T q_u);

    /**
     * @brief balances the disjoint interval sequence in L_in[0] and T_out[0] sequentially
     */
    void balance_v3_seq();

    // ############################# V3 PARALLEL #############################

    /**
     * @brief checks if an output interval is unbalanced and returns the a+1-th input interval starting in it
     * @param pln_I (p_i,q_i), [p_i, p_i + d_i - 1] must be the first input interval connected to [q_j, q_j + d_j - 1] in the permutation graph
     * @param ptn_J (p_j,q_j), [q_j, q_j + d_j - 1]
     * @param ptn_J_nxt (p_{j'},q_{j'}), with q_j + d_j = q_{j'}
     * @return (p_{i+a},q_{i+a}) if [q_j, q_j + d_j - 1] is unbalanced, else NULL
     */
    inline pair_list_node<T>* is_unbalanced_par(pair_list_node<T>* pln_I, pair_tree_node<T>* ptn_J, pair_tree_node<T>* ptn_J_nxt);

    /**
     * @brief balances the output interval [q_j, q_j + d_j - 1] by inserting the newly created pair into T_out[i_p] and Q_ins[0..p-1][i_p]
     * @param Q_ins reference to Q_ins
     * @param pln_IpA (p_{i+a},q_{i+a}), [p_i, p_i + d_i - 1] must be the first input interval connected to [q_j, q_j + d_j - 1] in the permutation graph
     * @param ptn_J (p_j,q_j), [q_j, q_j + d_j - 1] must be the first unbalanced output interval starting in [s[i_p]..q_u]
     * @param q_u starting position of an output interval starting in [s[i_p]..s[i_p+1]]
     * @return the newly created pair (p_j+d,q_j+d)
     */
    inline pair_tree_node<T>* insert_pair(ins_matr<T> &Q_ins, pair_list_node<T>* pln_IpA, pair_tree_node<T>* ptn_J, T q_u);

    /**
     * @brief balances the disjoint interval sequence in L_in[0..p-1] and T_out[0..p-1] in parallel
     */
    void balance_v3_par();

    // ############################# V4 PARALLEL #############################

    /**
     * @brief balances the output interval [q_j, q_j + d_j - 1] by inserting the newly created pair into T_out[i_p] and Q_ins[0..p-1][i_p]
     * @param Q_ins reference to Q_ins
     * @param D_inserted D_inserted[i] stores, whether thread i has already inserted all pairs, which the current thread has inserted into it's insert queue, into it's section
     * @param pln_IpA (p_{i+a},q_{i+a}), [p_i, p_i + d_i - 1] must be the first input interval connected to [q_j, q_j + d_j - 1] in the permutation graph
     * @param ptn_J (p_j,q_j), [q_j, q_j + d_j - 1] must be the first unbalanced output interval starting in [s[i_p]..q_u]
     * @param q_u starting position of an output interval starting in [s[i_p]..s[i_p+1]]
     * @return the newly created pair (p_j+d,q_j+d)
     */
    inline pair_tree_node<T>* insert_pair_conc(ins_matr_conc<T> &Q_ins, std::vector<bool> &D_inserted, pair_list_node<T> *pln_IpA, pair_tree_node<T> *ptn_J, T q_u);

    /**
     * @brief balances the disjoint interval sequence in L_in[0..p-1] and T_out[0..p-1] in parallel
     */
    void balance_v4_par();
};